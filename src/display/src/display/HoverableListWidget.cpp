#include <display/HoverableListWidget.h>
#include <finder/Dictionary.h>

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QToolTip>
#include <QUrl>
#ifdef Q_OS_WIN
#include <QProcess>
#endif

#include <utils/filesystem/filesystem.hpp>

HoverableListWidget::HoverableListWidget(QWidget *parent)
    : QListWidget(parent) {
  setMouseTracking(true);

  clickTimer.setSingleShot(true);

  // This logic stil has some issues for example clickeing two different items within double click duration... But oh well

  // Handle single-click logic when the timer times out
  connect(&clickTimer, &QTimer::timeout, this, [this]() {
    // save the pointer for edgecase that secons dingle click and time out occure parallel
    QListWidgetItem *item = pendingItem;
    pendingItem = nullptr;
    clickTimer.stop();
    if (item == nullptr) {
      return;
    }

    QString filePath = item->toolTip();  // Retrieve the full path from the tooltip
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists()) {
      QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    } else {
      QString folderPath = fileInfo.absolutePath();

#ifdef Q_OS_WIN
      QProcess::startDetached("explorer.exe",
                              {"/select,", QDir::toNativeSeparators(filePath)});
#else
          QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
#endif
    }
  });

  // Single-click detection (also triggers twice on double click, so use timer to distinguish)
  connect(this, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
    if (pendingItem != nullptr) {
      pendingItem = nullptr;
      // Double-click action logic
      QDesktopServices::openUrl(QUrl::fromLocalFile(item->toolTip()));
      return;
    }
    pendingItem = item;  // Store item for use in single-click action
    clickTimer.start(getDoubleClickInterval());
  });

#ifdef Q_OS_WIN
  // windows does not propergate the itemCLicked twice on a double click, linux does...
  connect(this, &QListWidget::itemDoubleClicked, [this](QListWidgetItem *item) {
    pendingItem = nullptr;
    QDesktopServices::openUrl(QUrl::fromLocalFile(item->toolTip()));
  });
#endif
}

void HoverableListWidget::mouseMoveEvent(QMouseEvent *event) {
  QListWidget::mouseMoveEvent(event);
}

void HoverableListWidget::leaveEvent(QEvent *event) {
  QListWidget::leaveEvent(event);
}

void HoverableListWidget::resizeEvent(QResizeEvent *event) {
  QListWidget::resizeEvent(event);

  setUpdatesEnabled(false);
  // Recalculate ellipses for all items based on the new size
  for (int i = 0; i < QListWidget::count(); ++i) {
    QListWidgetItem *item = QListWidget::item(i);
    const QString fullPath = shortenPathWithEllipsis(i) + entries[i].rightPathPartWithHtml;
    item->setText(fullPath);
  }

  setUpdatesEnabled(true);
  viewport()->update();
}

void HoverableListWidget::scrollContentsBy(int dx, int dy) {
  QListWidget::scrollContentsBy(dx, dy);

  if (dy < 0) {
    for (size_t i = lastVisibleIndex + 1; i < entries.size() && dy < 0; ++i, ++dy) {
      makeEntrieVisible(i);
      lastVisibleIndex = i;
    }
  }
}

int HoverableListWidget::getWordWidth(const QString &word) const {
  QFontMetrics fm = QPainter(viewport()).fontMetrics();
  return fm.width(word);
}

QString HoverableListWidget::shortenPathWithEllipsis(const size_t entryIndex) const {
  QFontMetrics metrics(font());
  constexpr int MAGIC_PADDING = 25;
  const int availableWidth = viewport()->width() - MAGIC_PADDING -
                             metrics.width(entries[entryIndex].rightPathPart);

  return metrics.elidedText(entries[entryIndex].leftPathPart, Qt::ElideMiddle, availableWidth);
}

size_t HoverableListWidget::getNumItemsBeforeScrollbar() const {
  if (QListWidget::count() > 0) {
    const int viewportHeight = viewport()->height();
    const int itemHeight =
        QListWidget::visualRect(QListWidget::model()->index(0, 0)).height();

    if (viewportHeight > 0 && itemHeight > 0) {
      return static_cast<size_t>(viewportHeight / itemHeight);
    }
  }
  return 42;
}

void HoverableListWidget::addSearchResultItem(const std::filesystem::path &searchResult) {

  QString path =
      QString::fromStdWString(searchResult.parent_path().wstring()) + "/";
  const std::wstring match = util::getLastPathComponent(searchResult);
  QString rightPathPart = QString::fromStdWString(match);
  // +5 just in case: we want to see a scrollbar to trigger the scroll event
  const bool visible = QListWidget::count() < getNumItemsBeforeScrollbar() + 5;
  entries.emplace_back(ItemInfo{path, rightPathPart, rightPathPart, visible});

  if (visible) {
    lastVisibleIndex = entries.size() - 1;
    makeEntrieVisible(lastVisibleIndex);
  }
}

void HoverableListWidget::makeEntrieVisible(const size_t entriyIndex) {
  static std::unordered_map<int, QString> scoreEmphasis = {
      {3, "color: green; font-weight: bold;"},    // Exact match
      {2, "color: green; font-weight: italic;"},  // Case mismatch
      {1, "color: blue;"},                        // Confused character
      {0, "color: black;"},                       // Total mismatch
  };
  assert(entries.size() > entriyIndex);

  const std::wstring match = entries[entriyIndex].rightPathPart.toStdWString();
  std::vector<int> scores = Dictionary::getMatchScores(searchInput, match);

  QString emphasizedMatch;
  for (size_t i = 0; i < match.size(); ++i) {
    QString style = scoreEmphasis[scores[i]];
    emphasizedMatch +=
        QString("<span style='%1'>%2</span>").arg(style).arg(QString(match[i]));
  }
  entries[entriyIndex].rightPathPartWithHtml = emphasizedMatch;


  QString fullPathStr = shortenPathWithEllipsis(entriyIndex) + emphasizedMatch;


  QListWidgetItem *item = new QListWidgetItem(fullPathStr);
  item->setToolTip(QString::fromStdWString(entries[entriyIndex].getPath().wstring()));
  item->setFlags(item->flags() | Qt::ItemIsSelectable);
  QFont font = QWidget::font();
  item->setFont(font);  // set font to listen to scale change
  QListWidget::addItem(item);
}

void HoverableListWidget::clear(const size_t newSize) {
  entries.clear();
  entries.reserve(newSize);
  lastVisibleIndex = 0;
  QListWidget::clear();
}

void HoverableListWidget::changeScale(const double scale_factor) {
  QFont font = this->font();
  font.setPointSizeF(font.pointSizeF() * scale_factor);
  setFont(font);  // Apply to all items

  // Notify the delegate/view to update item sizes
  updateGeometries();
  for (int i = 0; i < count(); ++i) {
    item(i)->setSizeHint(sizeHintForIndex(model()->index(i, 0)));
  }

  // Optionally, force a repaint to make sure changes appear immediately
  viewport()->update();
}
