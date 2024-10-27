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
#include <globals/macros.hpp>
#include <utils/filesystem/filesystem.hpp>



HoverableListWidget::HoverableListWidget(QWidget *parent)
    : QListWidget(parent) {
  setMouseTracking(true);

  // double click opens item
  connect(this, &QListWidget::itemDoubleClicked, [this](QListWidgetItem *item) {
    clickTimer.invalidate();
    DEBUG("double click");
    QString filePath = item->toolTip();  // Retrieve the full path from the tooltip
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
  });

  // single click opens folder beneath
  connect(this, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
    if (clickTimer.isValid()) {
      return;
    }
    clickTimer.start();  // Start timer for single click
    DEBUG("single click");
    // pause thread for DOUBLE_CLICK_INTERVAL

    while (clickTimer.isValid() && clickTimer.elapsed() < DOUBLE_CLICK_INTERVAL) {
    }
    DEBUG("single click timeout");
    if (!clickTimer.isValid()) {
      return;  // we had a double click
    }
    clickTimer.invalidate();
    DEBUG("single click valide");

    // Emit single click logic
    if (item) {
      QString filePath = item->toolTip();  // Retrieve the full path from the tooltip
      QFileInfo fileInfo(filePath);

      if (fileInfo.exists()) {
        QString folderPath = fileInfo.absolutePath();

        // For Windows, open folder and highlight file
#ifdef Q_OS_WIN
        /* QString command =
           QString("explorer.exe /select,%1").arg(QDir::toNativeSeparators(filePath));*/
        QProcess::startDetached(
            "explorer.exe", {"/select,", QDir::toNativeSeparators(filePath)});
#else
        // On other platforms, just open the folder
        QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
#endif
      } else {
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
      }
      emit itemClicked(item);
    }
  });
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
  const int availableWidth = viewport()->width() - 20 - metrics.width(rightPathPart);  // Consider some padding

  return metrics.elidedText(leftPathPart, Qt::ElideMiddle, availableWidth);
}



void HoverableListWidget::addSearchResultItem(const std::filesystem::path &searchResult,
                                              const std::string &search) {

  static std::unordered_map<int, QString> scoreEmphasis = {
      {0, "color: green; font-weight: bold;"},    // Exact match
      {1, "color: green; font-weight: italic;"},  // Case mismatch
      {2, "color: blue;"},                        // Confused character
      {3, "color: black;"},                       // Total mismatch
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
  addItem(item);
}

void HoverableListWidget::clear() {
  entries.clear();
  QListWidget::clear();
}
