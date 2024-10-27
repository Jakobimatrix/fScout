#pragma once

#include <QListWidget>
#include <QObject>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QTimer>
#include <filesystem>
#include <functional>
#include <map>
#include <vector>

class RichTextDelegate : public QStyledItemDelegate {
 public:
  using QStyledItemDelegate::QStyledItemDelegate;

  // Calculate item height based on HTML content
QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
    QTextDocument doc;
    doc.setDefaultFont(option.font);  // Use scaled font size
    doc.setHtml(index.data().toString());
    doc.setTextWidth(option.rect.width());  // Match document width to item width
    return QSize(option.rect.width(), doc.size().height());
}

  // Render HTML content with hover and selection effects
  void paint(QPainter *painter,
             const QStyleOptionViewItem &option,
             const QModelIndex &index) const override {
    if (index.data().canConvert<QString>()) {
      QString text = index.data().toString();
      QTextDocument doc;
      doc.setHtml(text);

      // use the font from parent
      QFont scaledFont = option.font;
      doc.setDefaultFont(scaledFont);

      // Set document width to the available item width
      doc.setTextWidth(option.rect.width());

      painter->save();

      // Set hover and selection background colors
      if (option.state.testFlag(QStyle::State_MouseOver)) {
        painter->fillRect(option.rect, QColor(211, 211, 211));  // Light gray for hover
      } else if (option.state.testFlag(QStyle::State_Selected)) {
        // painter->fillRect(option.rect, option.palette.highlight());  // Selection color
      }

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
  using GetDoubleClickInterval = std::function<int()>;

  HoverableListWidget(QWidget *parent = nullptr);
  void addSearchResultItem(const std::filesystem::path &searchResult,
                           const std::string &search);


  void clear();

  void setDoubleClickIntervalFunction(const GetDoubleClickInterval &func) {
    getDoubleClickInterval = func;
  }

  void changeScale(const double scale_factor);

 private:
  GetDoubleClickInterval getDoubleClickInterval = []() { return 255; };
  void mouseMoveEvent(QMouseEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  QString shortenPathWithEllipsis(const QString &leftPathPart,
                                  const QString &rightPathPart) const;

  int getWordWidth(const QString &word) const;

  QTimer clickTimer;
  QListWidgetItem *pendingItem = nullptr;

  struct ItemInfo {
    QString leftPathPart;
    QString rightPathPart;
    QString rightPathPartWithHtml;
  };

  std::vector<ItemInfo> entries;
};
