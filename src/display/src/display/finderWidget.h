#pragma once

#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QTextEdit>
#include <QWidget>

class DisplayQt;

class FinderWidget : public QWidget {
  Q_OBJECT

 public:
  explicit FinderWidget(QWidget *mommy);

  ~FinderWidget() override {}

  void reset();

  void updateInfo(const std::wstring &root_path,
                  const size_t num_files,
                  const std::wstring &indexing_date);



 protected:
 protected slots:

 private:
  std::wstring rootpath_default;
  DisplayQt *displayQt;
  QGroupBox *create_controlls();
  QGroupBox *create_info();
  QLabel *rootPathLabel = new QLabel();
  QLabel *filesFoundLabel = new QLabel();
  QLabel *indexingDate = new QLabel();
};
