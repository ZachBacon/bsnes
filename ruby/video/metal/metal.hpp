//
//  metal.hpp
//  ares
//
//  Created by jcm on 3/4/24.
//

#include <Metal/Metal.h>
#include <MetalKit/MTKView.h>

#include <librashader/librashader_ld.h>
#include "ShaderTypes.h"

struct Metal;

//macOS likes triple buffering, so only use three output buffers.
static const NSUInteger kMaxOutputBuffersInFlight = 3;

//If the frame queue gets above this depth, start discarding frames.
static const uint32_t kMaxSourceBuffersInFlight = 6;

/// MARK: VRR constants
/// These constants were picked through trial and error testing; not any objective principles. It may be possible to
/// achieve slight improvements, especially if optimizing for a particular system. These were found to generally perform
/// well across all of ares's systems that were tested. Despite thorough testing, the immediate present mode together
/// with the lowest possible audio latency may still be a better choice for some systems, particularly WonderSwan.

//How far back the rolling average of present intervals extends.
static const double kPresentIntervalRollingAverageWeight = 0.05;

//The amount that we must be off of the target refresh rate before we start 'nudging' it toward earlier presents; here, 0.5%.
static const double kVRRCorrectiveTolerance = .005;

//Amount that we "nudge" the present interval when we are behind the target refresh rate.
//In this way, we can sometimes achieve presents between 8ms and 16ms, leading to smoother pacing.
static const double kVRRCorrectiveForce = 5.0;

//If the frame queue gets above this depth, start scheduling immediate (0ms) presents.
static const uint32_t kVRRImmediatePresentThreshold = 3;

struct Metal {
  auto setShader(const string& pathname) -> void;
  auto clear() -> void;
  auto output() -> void;
  auto initialize(const string& shader) -> bool;
  auto terminate() -> void;
  auto refreshRateHint(double refreshRate) -> void;
  
  auto size(uint32_t width, uint32_t height) -> void;
  auto release() -> void;
  auto render(uint32_t sourceWidth, uint32_t sourceHeight, uint32_t targetX, uint32_t targetY, uint32_t targetWidth, uint32_t targetHeight) -> void;
  
  uint32_t *buffer = nullptr;
  
  uint32_t sourceWidth = 0;
  uint32_t sourceHeight = 0;
  uint32_t bytesPerRow = 0;
  
  uint32_t outputWidth = 0;
  uint32_t outputHeight = 0;
  double _outputX = 0;
  double _outputY = 0;
  uint32_t depth = 0;
  
  dispatch_queue_t _renderQueue = nullptr;
  
  CGFloat _viewWidth = 0;
  CGFloat _viewHeight = 0;
  vector_uint2 _viewportSize;
  
  CFTimeInterval _presentInterval = .016;
  CFTimeInterval averagePresentDuration = .016;
  CFTimeInterval previousPresentedTime = 0;
  uint32_t frameCount = 0;
  CFTimeInterval _refreshRateHint = 60;
  
  bool _blocking = false;
  bool _flush = false;
  bool _vrrIsSupported = false;
  bool _threaded = true;
  bool _nativeFullScreen = false;
  
  NSRect frameBeforeFullScreen = NSMakeRect(0,0,0,0);
  
  id<MTLDevice> _device;
  id<MTLCommandQueue> _commandQueue;
  id<MTLLibrary> _library;
  dispatch_semaphore_t _semaphore;
  
  id<MTLBuffer> _vertexBuffer;
  id<MTLTexture> _sourceTextures[kMaxSourceBuffersInFlight];
  MTLVertexDescriptor *_mtlVertexDescriptor;
  
  MTLRenderPassDescriptor *_renderToTextureRenderPassDescriptor;
  id<MTLRenderPipelineState> _renderToTextureRenderPipeline;
  id<MTLRenderPipelineState> _drawableRenderPipeline;
  id<MTLTexture> _renderTargetTexture;
  
  libra_instance_t _libra;
  libra_shader_preset_t _preset = nullptr;
  libra_mtl_filter_chain_t _filterChain = nullptr;
  bool initialized = false;
};