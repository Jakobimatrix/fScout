#pragma once

#include <display/HoverableListWidget.h>

#include <QGroupBox>
#include <QLineEdit>
#include <QListWidget>
#include <QObject>
#include <QTextEdit>
#include <QWidget>
#include <atomic>
#include <filesystem>
#include <functional>

class DisplayQt;

class FinderOutputWidget : public QWidget {
  Q_OBJECT

 public:
  explicit FinderOutputWidget(QWidget *parent);


  ~FinderOutputWidget() override {}

  void reset();

  void setSearchResults(const std::vector<std::filesystem::path> &searchResults,
                        const std::wstring &search);
  void changeScale(const double scaleFactor);

 protected:
 protected slots:

 private:
  QGroupBox *create_search();
  QGroupBox *create_resultField();
  HoverableListWidget *resultList;
  DisplayQt *displayQt;

  std::atomic<int> numQueuedProcesses = 0;
};
