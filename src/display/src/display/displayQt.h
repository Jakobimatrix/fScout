// shamelessly copied from https://doc.qt.io/archives/qt-4.8/qt-mainwindows-application-example.html
#pragma once


#include <QMainWindow>
#include <QObject>
#include <QSplitter>

#include "display.h"
#include "finderOutputWidget.h"
#include "finderWidget.h"

class DisplayQt : public QMainWindow, public Display {
  Q_OBJECT

 public:
  DisplayQt();

  ~DisplayQt() override = default;

 protected:
  void closeEvent(QCloseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void moveEvent(QMoveEvent *event) override;

  void setSearchResults(const std::vector<std::filesystem::path> &searchResults,
                        const std::wstring &search) override;

 private:
  void setStatus(const std::wstring &msg, int timeout = 0) override;
  void setStatus(const QString &msg, int timeout = 0);

  bool askYesNoQuestion(const std::wstring &question,
                        const std::wstring &title = L"") override;

  void popup_info(const std::wstring &text, const std::wstring &title = L"") override;

  void popup_warning(const std::wstring &text, const std::wstring &title = L"") override;

  void popup_error(const std::wstring &text, const std::wstring &title = L"") override;

  void setWindowFilePath(const std::wstring &file_path) override;

  void resetDisplayElements() override;

  void updateInfo(const std::wstring &root_path,
                  const size_t num_files,
                  const std::wstring &indexingDate) override;

  std::filesystem::path openDirChooserDialog() override;

  void loadSplitterState();

  void saveSplitterState();

 public slots:
  void onScaleChanged(const QString &scaleText);


 private slots:
  void about();
  void open();
  void close();
  void visualize();

 private:
  void createActions();
  void createMenus();
  void createToolBars();
  void createStatusBar();
  void changeScale(const int scale, const bool is_scale_on_load);
  const static QStringList getAvailableZoomLevels();
  // Function to determine the index of the current zoom level based on the stored zoom
  const static int getCurrentZoomLevelIndex(int currentZoom);

  // QT stuff
  static constexpr int BASE_ICON_SIZE = 32;

  QMenu *fileMenu;
  QMenu *helpMenu;
  QMenu *editMenu;
  QToolBar *fileToolBar;
  QToolBar *editToolBar;
  QAction *openAct;
  QAction *exitAct;
  QAction *aboutAct;
  QAction *visualizeAct;

  FinderWidget *finder_widget;
  FinderOutputWidget *finder_output_widget;
  QSplitter *splitter;
};
