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

  void setSearchResults(const std::vector<std::filesystem::path> &searchResults) override;

 private:
  void setStatus(const std::string &msg, int timeout = 0) override;
  void setStatus(const QString &msg, int timeout = 0);

  bool askYesNoQuestion(const std::string &question,
                        const std::string &title = "") override;

  void popup_info(const std::string &text, const std::string &title = "") override;

  void popup_warning(const std::string &text, const std::string &title = "") override;

  void popup_error(const std::string &text, const std::string &title = "") override;

  void setWindowFilePath(const std::string &file_path) override;

  void resetDisplayElements() override;

  void updateInfo(const std::string &root_path,
                  const bool saved,
                  const size_t num_files,
                  const std::string &indexingDate) override;

  std::filesystem::path openDirChooserDialog() override;
  std::filesystem::path filePickerDialog(const std::string &filePostfix) override;
  std::filesystem::path fileSaveDialog(const std::string &filePostfix) override;

  void loadSplitterState();

  void saveSplitterState();

 public slots:
  void onScaleChanged(const QString &scaleText);


 private slots:
  bool save();
  void run();
  void about();
  void open();
  void load();
  void close();

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
  std::map<QAction *, QString> actionIcons;

  QMenu *fileMenu;
  QMenu *editMenu;
  QMenu *runMenu;
  QMenu *helpMenu;
  QToolBar *fileToolBar;
  QToolBar *editToolBar;
  QToolBar *runToolBar;
  QAction *openAct;
  QAction *loadAct;
  QAction *saveAct;
  QAction *runAct;
  QAction *exitAct;
  QAction *aboutAct;

  FinderWidget *finder_widget;
  FinderOutputWidget *finder_output_widget;
  QSplitter *splitter;
};
