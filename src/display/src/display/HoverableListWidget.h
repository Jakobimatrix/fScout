#pragma once

#include <QListWidget>
#include <QObject>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QTimer>
#include <filesystem>
#include <functional>
#include <globals/globals.hpp>
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

      // Set hover background color
      if (option.state.testFlag(QStyle::State_MouseOver)) {
        painter->fillRect(option.rect, QColor(211, 211, 211));  // Light gray for hover
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
  void addSearchResultItem(const std::filesystem::path &searchResult);

  void setSearchInput(const std::wstring &needle) { searchInput = needle; }


  void clear(const size_t newSize);

  void setDoubleClickIntervalFunction(const GetDoubleClickInterval &func) {
    getDoubleClickInterval = func;
  }

  void changeScale(const double scale_factor);

 private:
  GetDoubleClickInterval getDoubleClickInterval = []() { return 255; };
  void mouseMoveEvent(QMouseEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void scrollContentsBy(int dx, int dy) override;

  QString shortenPathWithEllipsis(const size_t entriyIndex) const;

  void makeEntrieVisible(const size_t entriyIndex);

  int getWordWidth(const QString &word) const;

  size_t getNumItemsBeforeScrollbar() const;

  QTimer clickTimer;
  QListWidgetItem *pendingItem = nullptr;

  struct ItemInfo {
    QString leftPathPart;
    QString rightPathPart;
    QString rightPathPartWithHtml;
    bool visible;

    std::filesystem::path getPath() const {
      std::filesystem::path path(leftPathPart.toStdWString());
      return path / rightPathPart.toStdWString();
    }
  };

  std::vector<ItemInfo> entries;
  std::wstring searchInput;
  size_t lastVisibleIndex;
};
