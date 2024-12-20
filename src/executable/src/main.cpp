#include <display/displayQt.h>
#include <unistd.h>

#include <QAbstractEventDispatcher>
#include <QApplication>
#include <QSurfaceFormat>
#include <globals/globals.hpp>
#include <locale>

struct GPU {
  const int depth_buffer_size;
  const int stencil_buffer_size;
  const int openGL_mayor_version;
  const int openGL_minor_version;
};

constexpr GPU GPU_InterlHD4000{24, 8, 3, 0};

int main(int argc, char* argv[]) {

  // make sure to always use the same decimal point separator
  const auto locale = std::locale("C");
  printf("Set Settings output to locale setting: %s\n", locale.name().c_str());

  QApplication app(argc, argv);
  app.setApplicationName(Globals::getInstance().getMainWindowName().c_str());
  DisplayQt mainWin;
  QSurfaceFormat format;
  format.setDepthBufferSize(GPU_InterlHD4000.depth_buffer_size);
  format.setStencilBufferSize(GPU_InterlHD4000.stencil_buffer_size);
  format.setVersion(GPU_InterlHD4000.openGL_mayor_version, GPU_InterlHD4000.openGL_minor_version);
  format.setProfile(QSurfaceFormat::CompatibilityProfile);
  format.setRenderableType(QSurfaceFormat::OpenGL);
  format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
  QSurfaceFormat::setDefaultFormat(format);
  mainWin.show();
  /*
  QObject::connect(QAbstractEventDispatcher::instance(),
                   &QAbstractEventDispatcher::aboutToBlock,
                   []() { Globals::getFrameTimer().frameStart(); });

  QObject::connect(QAbstractEventDispatcher::instance(),
                   &QAbstractEventDispatcher::awake,
                   []() { Globals::getFrameTimer().frameStart(); });

  */
  return app.exec();
}
