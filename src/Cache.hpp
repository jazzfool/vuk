#pragma once
#include <shared_mutex>
#include <unordered_map>
#include <plf_colony.h>
#include "vuk/Hash.hpp"
#include "vuk/Types.hpp"
#include "vuk/Pipeline.hpp"
#include "vuk/Program.hpp"
#include "CreateInfo.hpp"
#include "RenderPass.hpp"
#include "RGImage.hpp"
#include <optional>
#include <span>
#include <vector>
#include "robin_hood.h" 
#include <atomic>

namespace std {
	template <>
	struct hash<VkVertexInputBindingDescription> {
		size_t operator()(VkVertexInputBindingDescription const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.binding, x.inputRate, x.stride);
			return h;
		}
	};

	template <>
	struct hash<VkVertexInputAttributeDescription> {
		size_t operator()(VkVertexInputAttributeDescription const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.binding, x.format, x.location, x.offset);
			return h;
		}
	};

	template <>
	struct hash<VkPipelineVertexInputStateCreateInfo> {
		size_t operator()(VkPipelineVertexInputStateCreateInfo const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.flags, std::span(x.pVertexBindingDescriptions, x.vertexBindingDescriptionCount), std::span(x.pVertexAttributeDescriptions, x.vertexAttributeDescriptionCount));
			return h;
		}
	};

	template <>
	struct hash<VkSpecializationMapEntry> {
		size_t operator()(VkSpecializationMapEntry const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.constantID, x.offset, x.size);
			return h;
		}
	};

	template <>
	struct hash<VkSpecializationInfo> {
		size_t operator()(VkSpecializationInfo const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, std::span(x.pMapEntries, x.mapEntryCount), std::span((std::byte*)x.pData, x.dataSize));
			return h;
		}
	};

	template <>
	struct hash<VkPipelineShaderStageCreateInfo> {
		size_t operator()(VkPipelineShaderStageCreateInfo const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.flags, x.pName, to_integral(x.stage), reinterpret_cast<uint64_t>((VkShaderModule)x.module));
			if (x.pSpecializationInfo) hash_combine(h, *x.pSpecializationInfo);
			return h;
		}
	};

	template <>
	struct hash<VkPipelineTessellationStateCreateInfo> {
		size_t operator()(VkPipelineTessellationStateCreateInfo const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.flags, x.patchControlPoints);
			return h;
		}
	};

	template <>
	struct hash<vuk::Extent2D> {
		size_t operator()(vuk::Extent2D const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.width, x.height);
			return h;
		}
	};

	template <>
	struct hash<vuk::Extent3D> {
		size_t operator()(vuk::Extent3D const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.width, x.height, x.depth);
			return h;
		}
	};

	template <>
	struct hash<vuk::Offset2D> {
		size_t operator()(vuk::Offset2D const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.x, x.y);
			return h;
		}
	};

	template <>
	struct hash<VkRect2D> {
		size_t operator()(VkRect2D const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.extent, x.offset);
			return h;
		}
	};

	template <>
	struct hash<VkExtent2D> {
		size_t operator()(VkExtent2D const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.width, x.height);
			return h;
		}
	};

	template <>
	struct hash<VkExtent3D> {
		size_t operator()(VkExtent3D const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.width, x.height, x.depth);
			return h;
		}
	};

	template <>
	struct hash<VkOffset2D> {
		size_t operator()(VkOffset2D const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.x, x.y);
			return h;
		}
	};

	template <>
	struct hash<VkViewport> {
		size_t operator()(VkViewport const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.x, x.y, x.width, x.height, x.minDepth, x.maxDepth);
			return h;
		}
	};

	template <>
	struct hash<VkPipelineViewportStateCreateInfo> {
		size_t operator()(VkPipelineViewportStateCreateInfo const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.flags);
			if (x.pScissors) hash_combine(h, std::span(x.pScissors, x.scissorCount));
			else hash_combine(h, x.scissorCount);
			if (x.pViewports) hash_combine(h, std::span(x.pViewports, x.viewportCount));
			else hash_combine(h, x.viewportCount);
			return h;
		}
	};

	template <>
	struct hash<VkPipelineMultisampleStateCreateInfo> {
		size_t operator()(VkPipelineMultisampleStateCreateInfo const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.flags, x.alphaToCoverageEnable, x.alphaToOneEnable, x.minSampleShading, x.rasterizationSamples, x.sampleShadingEnable);
			if (x.pSampleMask) hash_combine(h, *x.pSampleMask);
			return h;
		}
	};

	template <>
	struct hash<VkDynamicState> {
		size_t operator()(VkDynamicState const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, to_integral(x));
			return h;
		}
	};

	template <>
	struct hash<VkPipelineDynamicStateCreateInfo> {
		size_t operator()(VkPipelineDynamicStateCreateInfo const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.flags, std::span(x.pDynamicStates, x.dynamicStateCount));
			return h;
		}
	};

/*	template <>
	struct hash<VkGraphicsPipelineCreateInfo> {
		size_t operator()(VkGraphicsPipelineCreateInfo const & x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.flags, std::span(x.pStages, x.stageCount));
			if (x.pVertexInputState) hash_combine(h, *x.pVertexInputState);
			if (x.pInputAssemblyState) hash_combine(h, *x.pInputAssemblyState);
			if (x.pTessellationState) hash_combine(h, *x.pTessellationState);
			if (x.pViewportState) hash_combine(h, *x.pViewportState);
			if (x.pMultisampleState) hash_combine(h, *x.pMultisampleState);
			if (x.pDepthStencilState) hash_combine(h, *x.pDepthStencilState);
			if (x.pColorBlendState) hash_combine(h, *x.pColorBlendState);
			if (x.pDynamicState) hash_combine(h, *x.pDynamicState);
			hash_combine(h,
				reinterpret_cast<uint64_t>((VkPipelineLayout)x.layout),
				reinterpret_cast<uint64_t>((VkRenderPass)x.renderPass),
				x.subpass,
				reinterpret_cast<uint64_t>((VkPipeline)x.basePipelineHandle),
				x.basePipelineIndex);
			return h;
		}
	};*/
	
	template <>
	struct hash<VkAttachmentDescription> {
		size_t operator()(VkAttachmentDescription const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.flags, x.initialLayout, x.finalLayout, x.format, x.loadOp, x.stencilLoadOp, x.storeOp, x.stencilStoreOp, x.samples);
			return h;
		}
	};

	template <>
	struct hash<VkAttachmentReference> {
		size_t operator()(VkAttachmentReference const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.attachment, x.layout);
			return h;
		}
	};

	template <>
	struct hash<VkSubpassDependency> {
		size_t operator()(VkSubpassDependency const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.dependencyFlags, x.srcAccessMask, x.srcStageMask, x.srcSubpass, x.dstAccessMask, x.dstStageMask, x.dstSubpass);
			return h;
		}
	};

	template <>
	struct hash<vuk::ImageCreateInfo> {
		size_t operator()(vuk::ImageCreateInfo const & x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.flags, x.arrayLayers, x.extent, to_integral(x.format), to_integral(x.imageType), to_integral(x.initialLayout), x.mipLevels, std::span(x.pQueueFamilyIndices, x.queueFamilyIndexCount), to_integral(x.samples), to_integral(x.sharingMode), to_integral(x.tiling), x.usage);
			return h;
		}
	};
	
	template <>
	struct hash<vuk::ImageSubresourceRange> {
		size_t operator()(vuk::ImageSubresourceRange const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.aspectMask, x.baseArrayLayer, x.baseMipLevel, x.layerCount, x.levelCount);
			return h;
		}
	};

	template <>
	struct hash<vuk::ComponentMapping> {
		size_t operator()(vuk::ComponentMapping const& x) const noexcept {
			size_t h = 0;
			hash_combine(h, to_integral(x.r), to_integral(x.g), to_integral(x.b), to_integral(x.a));
			return h;
		}
	};

	template <>
	struct hash<vuk::ImageViewCreateInfo> {
		size_t operator()(vuk::ImageViewCreateInfo const & x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.flags, x.components, to_integral(x.format), 
				reinterpret_cast<uint64_t>((VkImage)x.image),
				x.subresourceRange, to_integral(x.viewType));
			return h;
		}
	};

	template <>
	struct hash<vuk::SamplerCreateInfo> {
		size_t operator()(vuk::SamplerCreateInfo const & x) const noexcept {
			size_t h = 0;
			hash_combine(h, x.flags, x.addressModeU, x.addressModeV, x.addressModeW, x.anisotropyEnable, x.borderColor, x.compareEnable, x.compareOp, x.magFilter, x.maxAnisotropy, x.maxLod, x.minFilter, x.minLod, x.mipLodBias, x.mipmapMode, x.unnormalizedCoordinates); 
			return h;
		}
	};
};

namespace vuk {
	template<class T>
	class Cache {
	private:
		struct LRUEntry {
			T* ptr;
			size_t last_use_frame;
		};

		Context& ctx;
		plf::colony<T> pool;
		robin_hood::unordered_map<create_info_t<T>, LRUEntry> lru_map; // possibly vector_map or an intrusive map
		std::shared_mutex cache_mtx;
	public:
		Cache(Context& ctx) : ctx(ctx) {}
		~Cache();

		std::optional<T> remove(const create_info_t<T>& ci) {
			std::unique_lock _(cache_mtx);
			auto it = lru_map.find(ci);
			if (it != lru_map.end()) {
				auto res = std::move(*it->second.ptr);
				pool.erase(pool.get_iterator_from_pointer(it->second.ptr));
				lru_map.erase(it);
				return res;
			}
			return {};
		}

		template<class Compare>
		std::optional<T> remove(Compare cmp) {
			std::unique_lock _(cache_mtx);
			for (auto it = lru_map.begin(); it != lru_map.end(); ++it) {
				if (cmp(it->first, it->second)) {
					auto res = std::move(*it->second.ptr);
					pool.erase(pool.get_iterator_from_pointer(it->second.ptr));
					lru_map.erase(it);
					return res;
				}
			}
			return {};
		}

        void remove_ptr(const T* ptr) {
            std::unique_lock _(cache_mtx);
            for(auto it = lru_map.begin(); it != lru_map.end(); ++it) {
                if(ptr == it->second.ptr) {
                    pool.erase(pool.get_iterator_from_pointer(it->second.ptr));
                    lru_map.erase(it);
                    return;
                }
            }
        }

		template<class Compare>
		const T* find(Compare cmp) {
			std::unique_lock _(cache_mtx);
			for (auto it = lru_map.begin(); it != lru_map.end(); ++it) {
				if (cmp(it->first, it->second)) {
					return it->second.ptr;
				}
			}
			return nullptr;
		}

		T& acquire(const create_info_t<T>& ci);

		struct PFView {
			InflightContext& ifc;
			Cache& cache;

			PFView(InflightContext& ifc, Cache<T>& cache) : ifc(ifc), cache(cache) {}
		};

		struct PFPTView {
			PerThreadContext& ptc;
			PFView& view;

			PFPTView(PerThreadContext& ptc, PFView& view) : ptc(ptc), view(view) {}
			T& acquire(const create_info_t<T>& ci);
			void collect(size_t threshold);
		};
	};

	template<class T, size_t FC>
	class PerFrameCache {
	private:
		friend class InflightContext;
		struct LRUEntry {
			T value;
			size_t last_use_frame;
		};

		Context& ctx;
		struct PerFrame {
			robin_hood::unordered_map<create_info_t<T>, LRUEntry> lru_map;
            std::array<std::vector<T>, 32> per_thread_append_v;
            std::array<std::vector<create_info_t<T>>, 32> per_thread_append_k;
			
			std::mutex cache_mtx;
		};
		std::array<PerFrame, FC> data;
	
	public:
		PerFrameCache(Context& ctx) : ctx(ctx) {}
		~PerFrameCache();

		struct PFView {
            InflightContext& ifc;
            PerFrameCache& cache;

            PFView(InflightContext& ifc, PerFrameCache& cache);
        };

		struct PFPTView {
			PerThreadContext& ptc;
			PFView& view;

			PFPTView(PerThreadContext& ptc, PFView& view) : ptc(ptc), view(view) {}
			T& acquire(const create_info_t<T>& ci);
			void collect(size_t threshold);
		};

	};
}
