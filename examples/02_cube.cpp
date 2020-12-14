#include "example_runner.hpp"
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

/* 02_cube
* In this example we will draw a cube, still with a single attachment, but using vertex, index and uniform buffers.
* The cube will spin around its Y axis, which we will achieve by changing the model matrix each frame.
* This examples showcases using scratch allocations, which only live for one frame.
*
* These examples are powered by the example framework, which hides some of the code required, as that would be repeated for each example.
* Furthermore it allows launching individual examples and all examples with the example same code.
* Check out the framework (example_runner_*) files if interested!
*/

namespace {
	// The Y rotation angle of our cube
	float angle = 0.f;
	// Generate vertices and indices for the cube
	auto box = util::generate_cube();

	vuk::Example x{
		.name = "02_cube",
		// Same setup as previously
		.setup = [](vuk::ExampleRunner& runner, vuk::InflightContext& ifc) {
			vuk::PipelineBaseCreateInfo pci;
			pci.add_shader(util::read_entire_file("../../examples/ubo_test.vert"), "ubo_test.vert");
			pci.add_shader(util::read_entire_file("../../examples/triangle_depthshaded.frag"), "triangle_depthshaded.frag");
			runner.context->create_named_pipeline("cube", pci);
		},
		.render = [](vuk::ExampleRunner& runner, vuk::InflightContext& ifc) {
			// We acquire a context specific to the thread we are on (PerThreadContext)
			auto ptc = ifc.begin();

			// Request a scratch buffer allocation with specific data
			// The context allocates a buffer which supports the desired use and is from the correct heap
			// And enqueues a transfer operation, which will copy the given data
			// Finally it returns a vuk::Buffer, which holds the info for the allocation
			// And a TransferStub, which can be used to query for the transfer status
			auto [bverts, stub1] = ptc.create_scratch_buffer(vuk::MemoryUsage::eGPUonly, vuk::BufferUsageFlagBits::eVertexBuffer, std::span(&box.first[0], box.first.size()));
			// We do this move here so that we can capture this variable later
			auto verts = std::move(bverts);
			auto [binds, stub2] = ptc.create_scratch_buffer(vuk::MemoryUsage::eGPUonly, vuk::BufferUsageFlagBits::eIndexBuffer, std::span(&box.second[0], box.second.size()));
			auto inds = std::move(binds);
			// This struct will represent the view-projection transform used for the cube
			struct VP {
				glm::mat4 view;
				glm::mat4 proj;
			} vp;
			// Fill the view matrix, looking a bit from top to the center
			vp.view = glm::lookAt(glm::vec3(0, 1.5, 3.5), glm::vec3(0), glm::vec3(0, 1, 0));
			// Fill the projection matrix, standard perspective matrix
			vp.proj = glm::perspective(glm::degrees(70.f), 1.f, 1.f, 10.f);
			vp.proj[1][1] *= -1;
			// Allocate and transfer view-projection transform
			auto [buboVP, stub3] = ptc.create_scratch_buffer(vuk::MemoryUsage::eCPUtoGPU, vuk::BufferUsageFlagBits::eUniformBuffer, std::span(&vp, 1));
			auto uboVP = buboVP;
			// For this example, we just request that all transfer finish before we continue
			ptc.wait_all_transfers();

			vuk::RenderGraph rg;
			rg.add_pass({
				// For this example, only a color image is needed to write to (our framebuffer)
				// The name is declared, and the way it will be used (color attachment - write)
				.resources = {"02_cube_final"_image(vuk::eColorWrite)},
				.execute = [verts, uboVP, inds](vuk::CommandBuffer& command_buffer) {
					command_buffer
					  .set_viewport(0, vuk::Rect2D::framebuffer()) // Set the viewport to cover the entire framebuffer
					  .set_scissor(0, vuk::Rect2D::framebuffer()) // Set the scissor area to cover the entire framebuffer
					  // The vertex format and the buffer used are bound together for this call
					  // The format is specified here as vuk::Packed{}, meaning we are going to make a consecutive binding
					  // For each element in the list, a vuk::Format signifies a binding
					  // And a vuk::Ignore signifies a number of bytes to be skipped
					  // In this case, we will bind vuk::Format::eR32G32B32Sfloat to the first location (0)
					  // And use the remaining vuk::Ignore-d bytes to establish the stride of the buffer
					  .bind_vertex_buffer(0, verts, 0, vuk::Packed{vuk::Format::eR32G32B32Sfloat, vuk::Ignore{sizeof(util::Vertex) - sizeof(util::Vertex::position)}})
					  // Bind the index buffer
					  .bind_index_buffer(inds, vuk::IndexType::eUint32)
					  .bind_graphics_pipeline("cube")
					  // Bind the uniform buffer we allocated to (set = 0, binding = 0)
					  .bind_uniform_buffer(0, 0, uboVP);
					  // For the model matrix, we will take a shorter route
					  // Frequently updated uniform buffers should always be in CPUtoGPU type memory, which is mappable
					  // So we create a typed mapping directly and write the model matrix
					glm::mat4* model = command_buffer.map_scratch_uniform_binding<glm::mat4>(0, 1);
					*model = static_cast<glm::mat4>(glm::angleAxis(glm::radians(angle), glm::vec3(0.f, 1.f, 0.f)));
					  // The cube is drawn via indexed drawing
					command_buffer
					  .draw_indexed(box.second.size(), 1, 0, 0, 0);
					}
				}
			);
			// The angle is update to rotate the cube
			angle += 360.f * ImGui::GetIO().DeltaTime;

			return rg;
		}
	};

	REGISTER_EXAMPLE(x);
}