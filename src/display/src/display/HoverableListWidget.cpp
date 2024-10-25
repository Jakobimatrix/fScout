#include <display/HoverableListWidget.h>

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QToolTip>
#include <QUrl>

HoverableListWidget::HoverableListWidget(QWidget *parent)
    : QListWidget(parent) {
  setMouseTracking(true);
}

void HoverableListWidget::mouseMoveEvent(QMouseEvent *event) {

  QListWidgetItem *hoveredItem = itemAt(event->pos());

  // Reset all items' background color
  for (int i = 0; i < count(); ++i) {
    QListWidgetItem *item = this->item(i);
    item->setBackground(Qt::NoBrush);
  }

  // Highlight the hovered item
  if (hoveredItem) {
    hoveredItem->setBackground(Qt::lightGray);
    // hoveredItem->toolTip().show();
  }

  QListWidget::mouseMoveEvent(event);
}

void HoverableListWidget::leaveEvent(QEvent *event) {
  for (int i = 0; i < count(); ++i) {
    QListWidgetItem *item = this->item(i);
    item->setBackground(Qt::NoBrush);
  }
  QListWidget::leaveEvent(event);
}

void HoverableListWidget::resizeEvent(QResizeEvent *event) {
  QListWidget::resizeEvent(event);

  // Recalculate ellipses for all items based on the new size
  for (int i = 0; i < count(); ++i) {
    QListWidgetItem *item = this->item(i);
    const QString fullPath = item->toolTip();
    item->setText(shortenPathWithEllipsis(fullPath));
  }
}

QString HoverableListWidget::shortenPathWithEllipsis(const QString &fullPath) const {
  QFontMetrics metrics(font());
  int availableWidth = viewport()->width() - 20;  // Consider some padding

  return metrics.elidedText(fullPath, Qt::ElideMiddle, availableWidth);
}

void HoverableListWidget::addSearchResultItem(const QString &fullPath) {
  QListWidgetItem *item = new QListWidgetItem(fullPath);
  item->setToolTip(fullPath);
  item->setFlags(item->flags() | Qt::ItemIsSelectable);
  addItem(item);
}
