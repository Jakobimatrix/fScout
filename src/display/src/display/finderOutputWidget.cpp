#include <display/displayQt.h>
#include <display/finderOutputWidget.h>

#include <QCheckBox>
#include <QCloseEvent>
#include <QCoreApplication>
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
#include <globals/timer.hpp>
#include <string>

FinderOutputWidget::FinderOutputWidget(QWidget *parent)
    : QWidget(parent), displayQt(reinterpret_cast<DisplayQt *>(parent)) {
  QVBoxLayout *layout = new QVBoxLayout();

  layout->addWidget(create_search());
  layout->addWidget(create_resultField());

  setLayout(layout);
}

QGroupBox *FinderOutputWidget::create_search() {
  QGroupBox *searchGroup = new QGroupBox("Search Files");
  QVBoxLayout *vbox = new QVBoxLayout(this);

  QLineEdit *searchField = new QLineEdit(this);
  connect(searchField, &QLineEdit::textChanged, [this](const QString &text) {
    displayQt->search(text.toStdWString());
  });
  vbox->addWidget(searchField);
  searchGroup->setLayout(vbox);
  return searchGroup;
}

QGroupBox *FinderOutputWidget::create_resultField() {
  QGroupBox *resultGroup = new QGroupBox("Search Results");

  QVBoxLayout *vbox = new QVBoxLayout(this);
  resultList = new HoverableListWidget(this);
  resultList->setToolTipDuration(10000);
  resultList->setItemDelegate(new RichTextDelegate(resultList));
  resultList->setDoubleClickIntervalFunction(
      std::bind(&Display::getDoubleClickInterval, displayQt));

  vbox->addWidget(resultList);
  resultGroup->setLayout(vbox);
  return resultGroup;
}


void FinderOutputWidget::setSearchResults(const std::vector<std::filesystem::path> &searchResults,
                                          const std::wstring &search) {

  if (QThread::currentThread() != this->thread()) {
    auto searchResultsCopy = searchResults;  // make a copy
    auto searchCopy = search;                // make a copy
    numQueuedProcesses.fetch_add(1);
    QMetaObject::invokeMethod(
        this,
        [this, searchResultsCopy, searchCopy]() {
          numQueuedProcesses.fetch_sub(1);
          setSearchResults(searchResultsCopy, searchCopy);
        },
        Qt::QueuedConnection);
    return;
  }

  // this will always be executed by QThread main. So no multithreading or painting problems here.

  resultList->clear(searchResults.size());
  resultList->setSearchInput(search);

  for (const auto &result : searchResults) {
    if (numQueuedProcesses.load() > 0) {
      return;  // we have new data
    }
    resultList->addSearchResultItem(result);
  }
}

void FinderOutputWidget::reset() {
  if (QThread::currentThread() != this->thread()) {
    QMetaObject::invokeMethod(
        this, [this]() { reset(); }, Qt::QueuedConnection);
    return;
  }
  resultList->clear(0);
}

void FinderOutputWidget::changeScale(const double scaleFactor) {
  resultList->changeScale(scaleFactor);
}
