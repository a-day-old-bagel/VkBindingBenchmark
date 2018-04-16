#include "Common.h"
#include "mesh_loading.h"
#include "rendering.h"
#include "camera.h"
#include "vkh.h"
#include "config.h"
#include <realtimeutils/topics.hpp>
#include <realtimeutils/keyInput.hpp>

/*
	Single threaded. Try to keep as much equal as possible, save for the experimental changes
	Test - binding once with uniform buffers, binding once with ssbo, binding per object uniform data

	- Read in all textures, store in a global array of textures, give materials push constant indices
	- meshes should be read in, buffers stored in a global place, but not combined
	- materials created by hand (since for this demo, we'll likely only have the one material, with a lot of instances)

*/

class VkbbState;
void logFPSAverage(double avg);
void mainLoop(VkbbState &state);
void cleanupSwapChain(VkbbState &state);
void cleanup(VkbbState &state);
void recreateSwapChain(VkbbState &state);

class VkbbState {

    std::unique_ptr<rtu::topics::Subscription> quitSub, mouseMove, keyW, keyA, keyS, keyD;
    float cameraSpeed = 2.f;
    float mouseSpeed = 0.005f;

  public:

    vkh::VkhContext context;
    std::vector<vkh::MeshAsset> testMesh;
    std::vector<uint32_t> uboIdx;
    Camera::Cam worldCamera;
    bool running = true;
    float leftRight = 0.f;
    float forwardBack = 0.f;

    VkbbState() {
      context.windowWidth = SCREEN_W;
      context.windowHeight = SCREEN_H;
      context.resizeDlgt = RTU_MTHD_DLGT(&VkbbState::onWindowResize, this);

      quitSub = std::make_unique<rtu::topics::Subscription>("quit", RTU_MTHD_DLGT(&VkbbState::onQuit, this));
      mouseMove = std::make_unique<rtu::topics::Subscription>("mouse_moved", RTU_MTHD_DLGT(&VkbbState::onMouse, this));
      keyW = std::make_unique<rtu::topics::Subscription>("key_held_w", RTU_MTHD_DLGT(&VkbbState::onKeyW, this));
      keyA = std::make_unique<rtu::topics::Subscription>("key_held_a", RTU_MTHD_DLGT(&VkbbState::onKeyA, this));
      keyS = std::make_unique<rtu::topics::Subscription>("key_held_s", RTU_MTHD_DLGT(&VkbbState::onKeyS, this));
      keyD = std::make_unique<rtu::topics::Subscription>("key_held_d", RTU_MTHD_DLGT(&VkbbState::onKeyD, this));

      vkh::VkhContextCreateInfo ctxtInfo = {};
      ctxtInfo.types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
      ctxtInfo.types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
      ctxtInfo.types.push_back(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
      ctxtInfo.types.push_back(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
      ctxtInfo.types.push_back(VK_DESCRIPTOR_TYPE_SAMPLER);
      ctxtInfo.typeCounts.push_back(512);
      ctxtInfo.typeCounts.push_back(512);
      ctxtInfo.typeCounts.push_back(512);
      ctxtInfo.typeCounts.push_back(512);
      ctxtInfo.typeCounts.push_back(512);
      ctxtInfo.typeCounts.push_back(1);
      initContext(ctxtInfo, "Uniform Buffer Array Demo", context);

      Camera::init(worldCamera);
    }

    void onQuit() { running = false; }

    void onMouse(void *eventPtr) {
      auto event = (SDL_Event *) eventPtr;
      Camera::rotate(worldCamera, glm::vec3(0.0f, 1.0f, 0.0f), (float) event->motion.xrel * -mouseSpeed);
      Camera::rotate(worldCamera, Camera::localRight(worldCamera), (float) event->motion.yrel * -mouseSpeed);
    }

    void onKeyW() { forwardBack += 1.f; }

    void onKeyS() { forwardBack -= 1.f; }

    void onKeyA() { leftRight += 1.f; }

    void onKeyD() { leftRight -= 1.f; }

    void onWindowResize() {
      recreateSwapChain(*this);
    }

    void tick(double dt) {
      glm::vec3 translation =
          (Camera::localForward(worldCamera) * forwardBack) + (Camera::localRight(worldCamera) * leftRight);
      Camera::translate(worldCamera, translation * cameraSpeed * (float) dt);
      forwardBack = 0.f;
      leftRight = 0.f;
    }
};

int main(int argc, char **argv) {

  VkbbState state;

  std::vector<vkh::EMeshVertexAttribute> meshLayout;
  meshLayout.push_back(vkh::EMeshVertexAttribute::POSITION);
  meshLayout.push_back(vkh::EMeshVertexAttribute::UV0);
  meshLayout.push_back(vkh::EMeshVertexAttribute::NORMAL);
  vkh::Mesh::setGlobalVertexLayout(meshLayout);

//load a test obj mesh
#if BISTRO_TEST
  state.testMesh = loadMesh("./meshes/exterior.obj", false, state.context);
  auto interior = loadMesh("./meshes/interior.obj", false, state.context);
  state.testMesh.insert(state.testMesh.end(), interior.begin(), interior.end());
#else
  state.testMesh = loadMesh("./meshes/sponza.obj", false, state.context);
#endif

  state.uboIdx.resize(state.testMesh.size());
  printf("Num meshes: %lu\n", state.testMesh.size());
  data_store::init(state.context);

  for (uint32_t i = 0; i < state.testMesh.size(); ++i) {
    bool didAcquire = data_store::acquire(state.uboIdx[i]);
    checkf(didAcquire, "Error acquiring ubo index");
  }

#if SHUFFLE_MESHES
  srand(8675309);
  for (uint32_t i = 0; i < state.testMesh.size(); ++i) {
    uint32_t newSlot = rand() % ((uint32_t)state.testMesh.size());
    std::swap(state.testMesh[i], state.testMesh[newSlot]);
    std::swap(state.uboIdx[i], state.uboIdx[newSlot]);
  }
#endif

  initRendering(state.context, state.testMesh.size());
  mainLoop(state);

  return 0;
}

void logFPSAverage(double avg) {
  printf("AVG FRAMETIME FOR LAST %i FRAMES: %f ms\n", FPS_DATA_FRAME_HISTORY_SIZE, avg);
}

void mainLoop(VkbbState &state) {
  FPSData fpsData = {0};
  fpsData.logCallback = logFPSAverage;
  startTimingFrame(fpsData);
  while (state.running) {
    double dt = endTimingFrame(fpsData);
    startTimingFrame(fpsData);
    sdl::pollEvents();
    state.tick(dt);
    render(state.context, state.worldCamera, state.testMesh, state.uboIdx);
  }
}

void cleanupSwapChain(VkbbState &state) {
  vkDestroyImageView(state.context.device, state.context.renderData.depthBuffer.view, nullptr);
  vkDestroyImage(state.context.device, state.context.renderData.depthBuffer.handle, nullptr);
  vkh::allocators::pool::free(state.context.renderData.depthBuffer.imageMemory);

  for (size_t i = 0; i < state.context.renderData.frameBuffers.size(); i++) {
    vkDestroyFramebuffer(state.context.device, state.context.renderData.frameBuffers[i], nullptr);
  }

  vkFreeCommandBuffers(state.context.device, state.context.gfxCommandPool,
                       static_cast<uint32_t>(state.context.renderData.commandBuffers.size()),
                       state.context.renderData.commandBuffers.data());

  vkDestroyPipeline(state.context.device, state.context.matData.graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(state.context.device, state.context.matData.pipelineLayout, nullptr);
  vkDestroyRenderPass(state.context.device, state.context.renderData.mainRenderPass, nullptr);

  for (size_t i = 0; i < state.context.swapChain.imageViews.size(); i++) {
    vkDestroyImageView(state.context.device, state.context.swapChain.imageViews[i], nullptr);
  }

  vkDestroySwapchainKHR(state.context.device, state.context.swapChain.swapChain, nullptr);
}

void recreateSwapChain(VkbbState &state) {
  printf("Recreating swap chain and reinitializing rendering routines.\n");

  int width, height;
  SDL_GetWindowSize(state.context.window, &width, &height);
  if (width <= 0 || height <= 0) {
    return;
  }

  state.context.windowWidth = width;
  state.context.windowHeight = height;

  vkDeviceWaitIdle(state.context.device);

  cleanupSwapChain(state);

  createSwapchainForSurface(state.context);
  initRendering(state.context, state.testMesh.size());
}
