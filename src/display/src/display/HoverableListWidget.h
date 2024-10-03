#pragma once

#include <QListWidget>
#include <QObject>

class HoverableListWidget : public QListWidget {
  Q_OBJECT

 public:
  HoverableListWidget(QWidget *parent = nullptr);
  void addSearchResultItem(const QString &fullPath);

 protected:
  void mouseMoveEvent(QMouseEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  QString shortenPathWithEllipsis(const QString &fullPath) const;
};
