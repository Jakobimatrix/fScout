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

  // Recalculate ellipses for all items based on the new size
  for (int i = 0; i < count(); ++i) {
    QListWidgetItem *item = this->item(i);
    const QString fullPath =
        shortenPathWithEllipsis(entries[i].leftPathPart, entries[i].rightPathPart) +
        entries[i].rightPathPartWithHtml;
    item->setText(fullPath);
  }
}

int HoverableListWidget::getWordWidth(const QString &word) const {
  QFontMetrics fm = QPainter(viewport()).fontMetrics();
  return fm.width(word);
}

QString HoverableListWidget::shortenPathWithEllipsis(const QString &leftPathPart,
                                                     const QString &rightPathPart) const {
  QFontMetrics metrics(font());
  constexpr int MAGIC_PADDING = 25;
  const int availableWidth =
      viewport()->width() - MAGIC_PADDING - metrics.width(rightPathPart);

  return metrics.elidedText(leftPathPart, Qt::ElideMiddle, availableWidth);
}



void HoverableListWidget::addSearchResultItem(const std::filesystem::path &searchResult,
                                              const std::string &search) {

  static std::unordered_map<int, QString> scoreEmphasis = {
      {3, "color: green; font-weight: bold;"},    // Exact match
      {2, "color: green; font-weight: italic;"},  // Case mismatch
      {1, "color: blue;"},                        // Confused character
      {0, "color: black;"},                       // Total mismatch
  };

  std::string match = util::getLastPathComponent(searchResult);
  std::vector<int> scores = Dictionary::getMatchScores(search, match);

  QString emphasizedMatch;
  for (size_t i = 0; i < match.size(); ++i) {
    QString style = scoreEmphasis[scores[i]];
    emphasizedMatch +=
        QString("<span style='%1'>%2</span>").arg(style).arg(QString(match[i]));
  }

  QString path =
      QString::fromStdString(searchResult.parent_path().string()) + "/";

  QString rightPathPart = QString::fromStdString(match);
  entries.emplace_back(ItemInfo{path, rightPathPart, emphasizedMatch});

  QString fullPathStr = shortenPathWithEllipsis(path, rightPathPart) + emphasizedMatch;


  QListWidgetItem *item = new QListWidgetItem(fullPathStr);
  item->setToolTip(searchResult.string().c_str());
  item->setFlags(item->flags() | Qt::ItemIsSelectable);
  QFont font = this->font();
  item->setFont(font);  // set font to listen to scale change
  addItem(item);
}

void HoverableListWidget::clear() {
  entries.clear();
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
