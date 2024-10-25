#pragma once

#include <QListWidget>
#include <QObject>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTextDocument>

class RichTextDelegate : public QStyledItemDelegate {
 public:
  using QStyledItemDelegate::QStyledItemDelegate;

  // Calculate item height based on HTML content
  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
    QTextDocument doc;
    doc.setHtml(index.data().toString());
    doc.setTextWidth(option.rect.width());  // Match document width to item width
    return QSize(option.rect.width(), doc.size().height());
  }

  // Render HTML content
  void paint(QPainter *painter,
             const QStyleOptionViewItem &option,
             const QModelIndex &index) const override {
    if (index.data().canConvert<QString>()) {
      QString text = index.data().toString();
      QTextDocument doc;
      doc.setHtml(text);

      // Set document width to the available item width
      doc.setTextWidth(option.rect.width());

      painter->save();
      painter->setClipRect(option.rect);
      painter->translate(option.rect.topLeft());  // Align at top-left
      doc.drawContents(painter);                  // Render full content
      painter->restore();
    } else {
      QStyledItemDelegate::paint(painter, option, index);
    }
  }
};

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
