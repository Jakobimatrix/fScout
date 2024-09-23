#pragma once

#include <QGroupBox>
#include <QLineEdit>
#include <QListWidget>
#include <QObject>
#include <QTextEdit>
#include <QWidget>
#include <filesystem>
#include <functional>

class DisplayQt;

class FinderOutputWidget : public QWidget {
  Q_OBJECT

 public:
  explicit FinderOutputWidget(QWidget *parent);


  ~FinderOutputWidget() override {}

  void reset();

  void setSearchResults(const std::vector<std::filesystem::path> &searchResults);

 protected:
 protected slots:

 private:
  QGroupBox *create_search();
  QGroupBox *create_resultField();
  QListWidget *resultList;
  DisplayQt *displayQt;
};
