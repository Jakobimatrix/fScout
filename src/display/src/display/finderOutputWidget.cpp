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
#include <string>

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
  resultList = new QListWidget;

  vbox->addWidget(resultList);
  resultGroup->setLayout(vbox);
  return resultGroup;
}


void FinderOutputWidget::setSearchResults(const std::vector<std::filesystem::path> &searchResults) {
  if (QThread::currentThread() != this->thread()) {
    // function was called from an worker thread. Delegate gui update to Main
    // Thread. Pass a copy of searchResults to the main thread
    auto searchResultsCopy = searchResults;  // make a copy
    QMetaObject::invokeMethod(
        this,
        [this, searchResultsCopy]() { setSearchResults(searchResultsCopy); },
        Qt::QueuedConnection);
    return;
  }

  // This will be executed by main Thread which updates the GUI correctly

  // Disconnect previous connections to avoid multiple openings
  disconnect(resultList, &QListWidget::itemClicked, nullptr, nullptr);

  resultList->clear();
  for (const auto &searchResult : searchResults) {
    QListWidgetItem *item = new QListWidgetItem(QString(searchResult.string().c_str()));
    resultList->addItem(item);
  }

  // Connect the itemClicked signal
  connect(resultList, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
    if (!displayQt->openFolderBeneath()) {
      QDesktopServices::openUrl(QUrl::fromLocalFile(item->text()));
      return;
    }
    QString filePath = item->text();
    QFileInfo fileInfo(filePath);

    if (fileInfo.exists()) {
      QString folderPath = fileInfo.absolutePath();

      // For Windows, you can open the folder and highlight the file
#ifdef Q_OS_WIN
      QString command =
          QString("explorer.exe /select,%1").arg(QDir::toNativeSeparators(filePath));
      QProcess::startDetached(command);
#else
        // On other platforms (Linux/Mac), just open the folder
        QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
#endif
    } else {
      QDesktopServices::openUrl(QUrl::fromLocalFile(item->text()));
    }
  });
}

void FinderOutputWidget::reset() { resultList->clear(); }
