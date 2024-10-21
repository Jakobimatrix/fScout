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
      QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(filePath)});
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


void FinderOutputWidget::setSearchResults(const std::vector<std::filesystem::path> &searchResults) {
  if (QThread::currentThread() != this->thread()) {
    auto searchResultsCopy = searchResults;  // make a copy
    QMetaObject::invokeMethod(
        this,
        [this, searchResultsCopy]() { setSearchResults(searchResultsCopy); },
        Qt::QueuedConnection);
    return;
  }

  resultList->clear();


  for (const auto &searchResult : searchResults) {
    QString fullPath = QString::fromStdString(searchResult.string());
    resultList->addSearchResultItem(fullPath);
  }
}

void FinderOutputWidget::reset() { resultList->clear(); }
