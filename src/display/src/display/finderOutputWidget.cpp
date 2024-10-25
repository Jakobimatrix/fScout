#include <display/displayQt.h>
#include <display/finderOutputWidget.h>

#include <QCheckBox>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QFocusEvent>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QResizeEvent>
#include <QShowEvent>
#include <QThread>
#include <QUrl>
#include <QVBoxLayout>
#include <globals/macros.hpp>
#include <string>
#include <utils/filesystem/filesystem.hpp>

#ifdef Q_OS_WIN
#include <QProcess>
#endif

FinderOutputWidget::FinderOutputWidget(QWidget *parent)
    : QWidget(parent), displayQt(reinterpret_cast<DisplayQt *>(parent)) {
  QVBoxLayout *layout = new QVBoxLayout;

  layout->addWidget(create_search());
  layout->addWidget(create_resultField());

  setLayout(layout);
}

QGroupBox *FinderOutputWidget::create_search() {
  QGroupBox *searchGroup = new QGroupBox("Search Files");
  QVBoxLayout *vbox = new QVBoxLayout;

  QLineEdit *searchField = new QLineEdit;
  connect(searchField, &QLineEdit::textChanged, [this](const QString &text) {
    displayQt->search(text.toStdString());
  });
  vbox->addWidget(searchField);

  searchGroup->setLayout(vbox);
  return searchGroup;
}

QGroupBox *FinderOutputWidget::create_resultField() {
  QGroupBox *resultGroup = new QGroupBox("Search Results");

  QVBoxLayout *vbox = new QVBoxLayout;
  resultList = new HoverableListWidget(this);
  resultList->setToolTipDuration(10000);
  resultList->setItemDelegate(new RichTextDelegate(resultList));

  connect(resultList, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
    DEBUG("connect on click");
    QString filePath = item->toolTip();  // Retrieve the full path from the tooltip
    DEBUG(filePath.toStdString().c_str());
    if (!displayQt->openFolderBeneath()) {
      QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
      return;
    }
    QFileInfo fileInfo(filePath);

    if (fileInfo.exists()) {
      QString folderPath = fileInfo.absolutePath();

      // For Windows, open folder and highlight file
#ifdef Q_OS_WIN
      /* QString command =
           QString("explorer.exe /select,%1").arg(QDir::toNativeSeparators(filePath));*/
      QProcess::startDetached("explorer.exe",
                              {"/select,", QDir::toNativeSeparators(filePath)});
#else
        // On other platforms, just open the folder
        QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
#endif
    } else {
      QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    }
  });

  vbox->addWidget(resultList);
  resultGroup->setLayout(vbox);
  return resultGroup;
}


void FinderOutputWidget::setSearchResults(const std::vector<std::filesystem::path> &searchResults,
                                          const std::string &search) {
  if (QThread::currentThread() != this->thread()) {
    auto searchResultsCopy = searchResults;  // make a copy
    auto searchCopy = search;                // make a copy
    QMetaObject::invokeMethod(
        this,
        [this, searchResultsCopy, searchCopy]() {
          setSearchResults(searchResultsCopy, searchCopy);
        },
        Qt::QueuedConnection);
    return;
  }

  resultList->clear();

  static std::unordered_map<int, QString> scoreEmphasis = {
      {0, "color: green; font-weight: bold;"},    // Exact match
      {1, "color: green; font-weight: italic;"},  // Case mismatch
      {2, "color: blue;"},                        // Confused character
      {3, "color: black;"},                       // Total mismatch
  };

  for (const std::filesystem::path &searchResult : searchResults) {
    std::string match = util::getLastPathComponent(searchResult);
    std::vector<int> scores = Dictionary::getMatchScores(search, match);

    QString emphasizedMatch;
    for (size_t i = 0; i < match.size(); ++i) {
      QString style = scoreEmphasis[scores[i]];
      emphasizedMatch +=
          QString("<span style='%1'>%2</span>").arg(style).arg(QString(match[i]));
    }

    // Build final HTML string for full path
    QString fullPathStr =
        QString::fromStdString(searchResult.parent_path().string()) + "/" + emphasizedMatch;

    // Add as rich text item
    resultList->addSearchResultItem(fullPathStr);
  }
}

void FinderOutputWidget::reset() { resultList->clear(); }
