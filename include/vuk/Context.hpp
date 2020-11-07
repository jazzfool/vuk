#pragma once

#include <atomic>
#include <span>

#include "Pool.hpp"
#include "Cache.hpp"
#include "Allocator.hpp"
#include "vuk/Program.hpp"
#include "vuk/Pipeline.hpp"
#include <queue>
#include <string_view>
#include "vuk/SampledImage.hpp"
#include "RenderPass.hpp"
#include "vuk_fwd.hpp"
#include <exception>
#include "vuk/Image.hpp"
#include "vuk/Buffer.hpp"

namespace vuk {
	struct RGImage {
		vuk::Image image;
		vuk::ImageView image_view;
	};
	struct RGCI {
		Name name;
		vuk::ImageCreateInfo ici;
		vuk::ImageViewCreateInfo ivci;

		bool operator==(const RGCI& other) const {
			return std::tie(name, ici, ivci) == std::tie(other.name, other.ici, other.ivci);
		}
	};
	template<> struct create_info<RGImage> {
		using type = RGCI;
	};

	struct ShaderCompilationException {
		std::string error_message;

		const char* what() const {
			return error_message.c_str();
		}
	};
}

namespace std {
	template <>
	struct hash<vuk::RGCI> {
		size_t operator()(vuk::RGCI const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.name, x.ici, x.ivci);
			return h;
		}
	};
};

namespace vuk {
	struct TransferStub {
		size_t id;
	};

	struct Swapchain {
		VkSwapchainKHR swapchain;
		VkSurfaceKHR surface;

		vuk::Format format;
		vuk::Extent2D extent = { 0, 0 };
		std::vector<vuk::Image> images;
		std::vector<VkImageView> _ivs;
		std::vector<vuk::ImageView> image_views;
	};
	using SwapchainRef = Swapchain*;
	struct Program;

	inline unsigned _prev(unsigned frame, unsigned amt, unsigned FC) {
		return ((frame - amt) % FC) + ((frame >= amt) ? 0 : FC - 1);
	}
	inline unsigned _next(unsigned frame, unsigned amt, unsigned FC) {
		return (frame + amt) % FC;
	}
	inline unsigned _next(unsigned frame, unsigned FC) {
		return (frame + 1) % FC;
	}
	inline size_t _next(size_t frame, unsigned FC) {
		return (frame + 1) % FC;
	}

	class Context {
	public:
		constexpr static size_t FC = 3;

		VkInstance instance;
		VkDevice device;
		VkPhysicalDevice physical_device;
		VkQueue graphics_queue;
		uint32_t graphics_queue_family_index;
		VkQueue transfer_queue;
		uint32_t transfer_queue_family_index;
		Allocator allocator;

		std::mutex gfx_queue_lock;
		std::mutex xfer_queue_lock;
	private:
		Pool<VkCommandBuffer, FC> cbuf_pools;
		Pool<VkSemaphore, FC> semaphore_pools;
		Pool<VkFence, FC> fence_pools;
		VkPipelineCache vk_pipeline_cache;
		Cache<PipelineBaseInfo> pipelinebase_cache;
		Cache<PipelineInfo> pipeline_cache;
		Cache<ComputePipelineInfo> compute_pipeline_cache;
		Cache<VkRenderPass> renderpass_cache;
		Cache<VkFramebuffer> framebuffer_cache;
		PerFrameCache<RGImage, FC> transient_images;
		PerFrameCache<Allocator::Linear, FC> scratch_buffers;
		Cache<vuk::DescriptorPool> pool_cache;
		PerFrameCache<vuk::DescriptorSet, FC> descriptor_sets;
		Cache<vuk::Sampler> sampler_cache;
		Pool<vuk::SampledImage, FC> sampled_images;
		Cache<vuk::ShaderModule> shader_modules;
		Cache<vuk::DescriptorSetLayoutAllocInfo> descriptor_set_layouts;
		Cache<VkPipelineLayout> pipeline_layouts;

		std::mutex begin_frame_lock;

		std::array<std::mutex, FC> recycle_locks;
		std::array<std::vector<vuk::Image>, FC> image_recycle;
		std::array<std::vector<VkImageView>, FC> image_view_recycle;
		std::array<std::vector<VkPipeline>, FC> pipeline_recycle;
		std::array<std::vector<vuk::Buffer>, FC> buffer_recycle;
		std::array<std::vector<vuk::PersistentDescriptorSet>, FC> pds_recycle;

		std::atomic<size_t> frame_counter = 0;
		std::atomic<size_t> unique_handle_id_counter = 0;

		std::mutex named_pipelines_lock;
		std::unordered_map<std::string_view, vuk::PipelineBaseInfo*> named_pipelines;
		std::unordered_map<std::string_view, vuk::ComputePipelineInfo*> named_compute_pipelines;

		std::mutex swapchains_lock;
		plf::colony<Swapchain> swapchains;
	public:
		Context(VkInstance instance, VkDevice device, VkPhysicalDevice physical_device, VkQueue graphics);
		~Context();

		struct DebugUtils {
			Context& ctx;
			PFN_vkSetDebugUtilsObjectNameEXT setDebugUtilsObjectNameEXT;
			PFN_vkCmdBeginDebugUtilsLabelEXT cmdBeginDebugUtilsLabelEXT;
			PFN_vkCmdEndDebugUtilsLabelEXT cmdEndDebugUtilsLabelEXT;

			bool enabled();

			DebugUtils(Context& ctx);
			void set_name(const vuk::Texture& iv, /*zstring_view*/Name name);
			template<class T>
			void set_name(const T& t, /*zstring_view*/Name name);

			void begin_region(const VkCommandBuffer&, Name name, std::array<float, 4> color = { 1,1,1,1 });
			void end_region(const VkCommandBuffer&);
		} debug;

		void create_named_pipeline(const char* name, vuk::PipelineBaseCreateInfo pbci);
		void create_named_pipeline(const char* name, vuk::ComputePipelineCreateInfo pbci);

		vuk::PipelineBaseInfo* get_named_pipeline(const char* name);
		vuk::ComputePipelineInfo* get_named_compute_pipeline(const char* name);

		vuk::PipelineBaseInfo* get_pipeline(const vuk::PipelineBaseCreateInfo& pbci);
		vuk::ComputePipelineInfo* get_pipeline(const vuk::ComputePipelineCreateInfo& pbci);
		vuk::Program get_pipeline_reflection_info(vuk::PipelineBaseCreateInfo pbci);
		vuk::ShaderModule compile_shader(std::string source, Name path);

		vuk::ShaderModule create(const create_info_t<vuk::ShaderModule>& cinfo);
		vuk::PipelineBaseInfo create(const create_info_t<vuk::PipelineBaseInfo>& cinfo);
		VkPipelineLayout create(const create_info_t<VkPipelineLayout>& cinfo);
		vuk::DescriptorSetLayoutAllocInfo create(const create_info_t<vuk::DescriptorSetLayoutAllocInfo>& cinfo);
		vuk::ComputePipelineInfo create(const create_info_t<vuk::ComputePipelineInfo>& cinfo);

		bool load_pipeline_cache(std::span<uint8_t> data);
		std::vector<uint8_t> save_pipeline_cache();

		// one pool per thread
		std::mutex one_time_pool_lock;
		std::vector<VkCommandPool> xfer_one_time_pools;
		std::vector<VkCommandPool> one_time_pools;

		uint32_t(*get_thread_index)() = nullptr;

		// when the fence is signaled, caller should clean up the resources
		struct UploadResult {
			VkFence fence;
			VkCommandBuffer command_buffer;
			vuk::Buffer staging;
			bool is_buffer;
			unsigned thread_index;
		};

		struct BufferUpload {
			vuk::Buffer dst;
			std::span<unsigned char> data;
		};
		UploadResult fenced_upload(std::span<BufferUpload>);

		struct ImageUpload {
			vuk::Image dst;
			vuk::Extent3D extent;
			std::span<unsigned char> data;
		};
		UploadResult fenced_upload(std::span<ImageUpload>);
		void free_upload_resources(const UploadResult&);

		Buffer allocate_buffer(MemoryUsage mem_usage, BufferUsageFlags buffer_usage, size_t size, size_t alignment);
		Texture allocate_texture(vuk::ImageCreateInfo ici);

		void enqueue_destroy(vuk::Image);
		void enqueue_destroy(vuk::ImageView);
		void enqueue_destroy(VkPipeline);
		void enqueue_destroy(vuk::Buffer);
		void enqueue_destroy(vuk::PersistentDescriptorSet);

		template<class T>
		Handle<T> wrap(T payload);

		SwapchainRef add_swapchain(Swapchain sw);

		InflightContext begin();

		void wait_idle();
	private:
		void destroy(const RGImage& image);
		void destroy(const Allocator::Pool& v);
		void destroy(const Allocator::Linear& v);
		void destroy(const vuk::DescriptorPool& dp);
		void destroy(const vuk::PipelineInfo& pi);
		void destroy(const vuk::ShaderModule& sm);
		void destroy(const vuk::DescriptorSetLayoutAllocInfo& ds);
		void destroy(const VkPipelineLayout& pl);
		void destroy(const VkRenderPass& rp);
		void destroy(const vuk::DescriptorSet&);
		void destroy(const VkFramebuffer& fb);
		void destroy(const vuk::Sampler& sa);
		void destroy(const vuk::PipelineBaseInfo& pbi);

		friend class InflightContext;
		friend class PerThreadContext;
		template<class T> friend class Cache; // caches can directly destroy
		template<class T, size_t FC> friend class PerFrameCache;
	};

	class InflightContext {
	public:
		Context& ctx;
		const size_t absolute_frame;
		const unsigned frame;
	private:
		Pool<VkFence, Context::FC>::PFView fence_pools; // must be first, so we wait for the fences
		Pool<VkCommandBuffer, Context::FC>::PFView commandbuffer_pools;
		Pool<VkSemaphore, Context::FC>::PFView semaphore_pools;
		Cache<PipelineInfo>::PFView pipeline_cache;
		Cache<ComputePipelineInfo>::PFView compute_pipeline_cache;
		Cache<PipelineBaseInfo>::PFView pipelinebase_cache;
		Cache<VkRenderPass>::PFView renderpass_cache;
		Cache<VkFramebuffer>::PFView framebuffer_cache;
		PerFrameCache<vuk::RGImage, Context::FC>::PFView transient_images;
		PerFrameCache<Allocator::Linear, Context::FC>::PFView scratch_buffers;
		PerFrameCache<vuk::DescriptorSet, Context::FC>::PFView descriptor_sets;
		Cache<vuk::Sampler>::PFView sampler_cache;
	public:
		Pool<vuk::SampledImage, Context::FC>::PFView sampled_images;
	private:
		Cache<vuk::DescriptorPool>::PFView pool_cache;

		Cache<vuk::ShaderModule>::PFView shader_modules;
		Cache<vuk::DescriptorSetLayoutAllocInfo>::PFView descriptor_set_layouts;
		Cache<VkPipelineLayout>::PFView pipeline_layouts;
	public:
		InflightContext(Context& ctx, size_t absolute_frame, std::lock_guard<std::mutex>&& recycle_guard);

		void wait_all_transfers();
		PerThreadContext begin();

		struct BufferCopyCommand {
			Buffer src;
			Buffer dst;
			TransferStub stub;
		};

		struct BufferImageCopyCommand {
			Buffer src;
			vuk::Image dst;
			vuk::Extent3D extent;
			bool generate_mips;
			TransferStub stub;
		};

	private:
		friend class PerThreadContext;

		std::atomic<size_t> transfer_id = 1;
		std::atomic<size_t> last_transfer_complete = 0;

		struct PendingTransfer {
			size_t last_transfer_id;
			VkFence fence;
		};
		// needs to be mpsc
		std::mutex transfer_mutex;
		std::queue<BufferCopyCommand> buffer_transfer_commands;
		std::queue<BufferImageCopyCommand> bufferimage_transfer_commands;
		// only accessed by DMAtask
		std::queue<PendingTransfer> pending_transfers;

		TransferStub enqueue_transfer(Buffer src, Buffer dst);
		TransferStub enqueue_transfer(Buffer src, vuk::Image dst, vuk::Extent3D extent, bool generate_mips);

		// recycle
		std::mutex recycle_lock;
		void destroy(std::vector<vuk::Image>&& images);
		void destroy(std::vector<VkImageView>&& images);
	};

	class PerThreadContext {
	public:
		Context& ctx;
		InflightContext& ifc;
		const unsigned tid = 0; // not yet implemented
		Pool<VkCommandBuffer, Context::FC>::PFPTView commandbuffer_pool;
		Pool<VkSemaphore, Context::FC>::PFPTView semaphore_pool;
		Pool<VkFence, Context::FC>::PFPTView fence_pool;
		Cache<PipelineInfo>::PFPTView pipeline_cache;
		Cache<ComputePipelineInfo>::PFPTView compute_pipeline_cache;
		Cache<PipelineBaseInfo>::PFPTView pipelinebase_cache;
		Cache<VkRenderPass>::PFPTView renderpass_cache;
		Cache<VkFramebuffer>::PFPTView framebuffer_cache;
		PerFrameCache<vuk::RGImage, Context::FC>::PFPTView transient_images;
		PerFrameCache<Allocator::Linear, Context::FC>::PFPTView scratch_buffers;
		PerFrameCache<vuk::DescriptorSet, Context::FC>::PFPTView descriptor_sets;
		Cache<vuk::Sampler>::PFPTView sampler_cache;
		Pool<vuk::SampledImage, Context::FC>::PFPTView sampled_images;
		Cache<vuk::DescriptorPool>::PFPTView pool_cache;
		Cache<vuk::ShaderModule>::PFPTView shader_modules;
		Cache<vuk::DescriptorSetLayoutAllocInfo>::PFPTView descriptor_set_layouts;
		Cache<VkPipelineLayout>::PFPTView pipeline_layouts;
	private:
		// recycling global objects
		std::vector<Buffer> buffer_recycle;
		std::vector<vuk::Image> image_recycle;
		std::vector<VkImageView> image_view_recycle;
	public:
		PerThreadContext(InflightContext& ifc, unsigned tid);
		~PerThreadContext();

		PerThreadContext(const PerThreadContext& o) = delete;

		bool is_ready(const TransferStub& stub);
		void wait_all_transfers();

		Unique<PersistentDescriptorSet> create_persistent_descriptorset(const PipelineBaseInfo& base, unsigned set, unsigned num_descriptors);
		Unique<PersistentDescriptorSet> create_persistent_descriptorset(const ComputePipelineInfo& base, unsigned set, unsigned num_descriptors);
		Unique<PersistentDescriptorSet> create_persistent_descriptorset(const DescriptorSetLayoutAllocInfo& dslai, unsigned num_descriptors);
		void commit_persistent_descriptorset(PersistentDescriptorSet& array);

		size_t get_allocation_size(Buffer);
		Buffer _allocate_scratch_buffer(MemoryUsage mem_usage, vuk::BufferUsageFlags buffer_usage, size_t size, size_t alignment, bool create_mapped);
		Unique<Buffer> _allocate_buffer(MemoryUsage mem_usage, vuk::BufferUsageFlags buffer_usage, size_t size, size_t alignment, bool create_mapped);

		// since data is provided, we will add TransferDst to the flags automatically
		template<class T>
		std::pair<Buffer, TransferStub> create_scratch_buffer(MemoryUsage mem_usage, vuk::BufferUsageFlags buffer_usage, std::span<T> data) {
			auto dst = _allocate_scratch_buffer(mem_usage, vuk::BufferUsageFlagBits::eTransferDst | buffer_usage, sizeof(T) * data.size(), 1, false);
			auto stub = upload(dst, data);
			return { dst, stub };
		}

		template<class T>
		std::pair<Unique<Buffer>, TransferStub> create_buffer(MemoryUsage mem_usage, vuk::BufferUsageFlags buffer_usage, std::span<T> data) {
			auto dst = _allocate_buffer(mem_usage, vuk::BufferUsageFlagBits::eTransferDst | buffer_usage, sizeof(T) * data.size(), 1, false);
			auto stub = upload(*dst, data);
			return { std::move(dst), stub };
		}


		vuk::Texture allocate_texture(vuk::ImageCreateInfo);
		std::pair<vuk::Texture, TransferStub> create_texture(vuk::Format format, vuk::Extent3D extents, void* data);

		template<class T>
		TransferStub upload(Buffer dst, std::span<T> data) {
			if (data.empty()) return { 0 };
			auto staging = _allocate_scratch_buffer(MemoryUsage::eCPUonly, vuk::BufferUsageFlagBits::eTransferSrc, sizeof(T) * data.size(), 1, true);
			::memcpy(staging.mapped_ptr, data.data(), sizeof(T) * data.size());

			return ifc.enqueue_transfer(staging, dst);
		}

		template<class T>
		TransferStub upload(vuk::Image dst, vuk::Extent3D extent, std::span<T> data, bool generate_mips) {
			assert(!data.empty());
			auto staging = _allocate_scratch_buffer(MemoryUsage::eCPUonly, vuk::BufferUsageFlagBits::eTransferSrc, sizeof(T) * data.size(), 1, true);
			::memcpy(staging.mapped_ptr, data.data(), sizeof(T) * data.size());

			return ifc.enqueue_transfer(staging, dst, extent, generate_mips);
		}

		void dma_task();

		vuk::SampledImage& make_sampled_image(vuk::ImageView iv, vuk::SamplerCreateInfo sci);
		vuk::SampledImage& make_sampled_image(Name n, vuk::SamplerCreateInfo sci);
		vuk::SampledImage& make_sampled_image(Name n, vuk::ImageViewCreateInfo ivci, vuk::SamplerCreateInfo sci);

		vuk::Program get_pipeline_reflection_info(vuk::PipelineBaseCreateInfo pci);

		template<class T>
		void destroy(const T& t) {
			ctx.destroy(t);
		}

		void destroy(vuk::Image image);
		void destroy(vuk::ImageView image);
		void destroy(vuk::DescriptorSet ds);

		PipelineBaseInfo create(const create_info_t<PipelineBaseInfo>& cinfo);
		PipelineInfo create(const create_info_t<PipelineInfo>& cinfo);
		vuk::ShaderModule create(const create_info_t<vuk::ShaderModule>& cinfo);
		VkRenderPass create(const create_info_t<VkRenderPass>& cinfo);
		vuk::RGImage create(const create_info_t<vuk::RGImage>& cinfo);
		//vuk::Allocator::Pool create(const create_info_t<vuk::Allocator::Pool>& cinfo);
		vuk::Allocator::Linear create(const create_info_t<vuk::Allocator::Linear>& cinfo);
		vuk::DescriptorPool create(const create_info_t<vuk::DescriptorPool>& cinfo);
		vuk::DescriptorSet create(const create_info_t<vuk::DescriptorSet>& cinfo);
		VkFramebuffer create(const create_info_t<VkFramebuffer>& cinfo);
		vuk::Sampler create(const create_info_t<vuk::Sampler>& cinfo);
		vuk::DescriptorSetLayoutAllocInfo create(const create_info_t<vuk::DescriptorSetLayoutAllocInfo>& cinfo);
		VkPipelineLayout create(const create_info_t<VkPipelineLayout>& cinfo);
		vuk::ComputePipelineInfo create(const create_info_t<vuk::ComputePipelineInfo>& cinfo);
	};

	template<class T>
	void Context::DebugUtils::set_name(const T& t, Name name) {
		if (!enabled()) return;
		VkDebugUtilsObjectNameInfoEXT info = { .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
		info.pObjectName = name.data();
		if constexpr (std::is_same_v<T, VkImage>) {
			info.objectType = VK_OBJECT_TYPE_IMAGE;
		} else if constexpr (std::is_same_v<T, VkImageView>) {
			info.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
		} else if constexpr (std::is_same_v<T, VkShaderModule>) {
			info.objectType = VK_OBJECT_TYPE_SHADER_MODULE;
		} else if constexpr (std::is_same_v<T, VkPipeline>) {
			info.objectType = VK_OBJECT_TYPE_PIPELINE;
		}
		//info.objectType = (VkObjectType)t.objectType;
		info.objectHandle = reinterpret_cast<uint64_t>(t);
		setDebugUtilsObjectNameEXT(ctx.device, &info);
	}

	template<class T>
	Handle<T> Context::wrap(T payload) {
		return { { unique_handle_id_counter++ }, payload };
	}
}

namespace vuk {
	template<class T, size_t FC>
	typename Pool<T, FC>::PFView Pool<T, FC>::get_view(InflightContext& ctx) {
		return { ctx, *this, per_frame_storage[ctx.frame] };
	}

	template<class T, size_t FC>
	Pool<T, FC>::PFView::PFView(InflightContext& ifc, Pool<T, FC>& storage, plf::colony<PooledType<T>>& fv) : storage(storage), ifc(ifc), frame_values(fv) {
		storage.reset(ifc.frame);
	}

	template<typename Type>
	inline Unique<Type>::~Unique() noexcept {
		if (context && payload != Type{})
			context->enqueue_destroy(std::move(payload));
	}
	template<typename Type>
	inline void Unique<Type>::reset(Type value) noexcept {
		if (payload != value) {
			if (context && payload != Type{}) {
				context->enqueue_destroy(std::move(payload));
			}
			payload = std::move(value);
		}
	}

	struct RenderGraph;
	bool execute_submit_and_present_to_one(PerThreadContext& ptc, RenderGraph& rg, SwapchainRef swapchain, bool use_secondary_command_buffers = false);
	void execute_submit_and_wait(PerThreadContext& ptc, RenderGraph& rg, bool use_secondary_command_buffers = false);
}