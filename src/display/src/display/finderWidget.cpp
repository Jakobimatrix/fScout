#include <display/displayQt.h>
#include <display/finderWidget.h>

#include <QCheckBox>
#include <QCloseEvent>
#include <QFocusEvent>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QShowEvent>
#include <QSpinBox>
#include <QThread>
#include <QVBoxLayout>

FinderWidget::FinderWidget(QWidget *mommy)
    : QWidget(mommy), displayQt(reinterpret_cast<DisplayQt *>(mommy)) {
  QVBoxLayout *layout = new QVBoxLayout;

  // Create Info section
  layout->addWidget(create_info());

  // Create Controls section
  layout->addWidget(create_controlls());

  setLayout(layout);
}

QGroupBox *FinderWidget::create_info() {
  QGroupBox *infoGroup = new QGroupBox("Indexing Information");
  QFormLayout *formLayout = new QFormLayout;

  formLayout->addRow(new QLabel("Root Path:"), rootPathLabel);
  formLayout->addRow(new QLabel("Files Found:"), filesFoundLabel);
  formLayout->addRow(new QLabel("Indexing Date:"), indexingDate);

  infoGroup->setLayout(formLayout);
  return infoGroup;
}

QGroupBox *FinderWidget::create_controlls() {
  QGroupBox *controlsGroup = new QGroupBox("Search Patterns");

  QGridLayout *grid = new QGridLayout;

  // Checkboxes for search patterns
  QCheckBox *exactSearchBox = new QCheckBox("Exact Search");
  QCheckBox *fuzzySearchBox = new QCheckBox("Fuzzy Search");
  QCheckBox *wildcardSearchBox = new QCheckBox("Wildcard (*) Search");
  QLineEdit *wildcardCharInput = new QLineEdit;
  wildcardCharInput->setMaxLength(1);
  wildcardCharInput->setText(QString(displayQt->getWildcardChar()));

  QCheckBox *subsearchSearchBox =
      new QCheckBox("Use Substrings of the search needle");
  QSpinBox *subsearchLengthInput = new QSpinBox;
  subsearchLengthInput->setMinimum(2);
  subsearchLengthInput->setValue(displayQt->getMinSubsearchSize());

  exactSearchBox->setChecked(displayQt->usesExactPattern());
  fuzzySearchBox->setChecked(displayQt->usesFuzzyMatchPattern());
  wildcardSearchBox->setChecked(displayQt->usesWildcardPattern());
  subsearchSearchBox->setChecked(displayQt->usesSubsearchPattern());

  // Connect signals to displayQt for search patterns
  connect(exactSearchBox, &QCheckBox::stateChanged, displayQt, [this](int state) {
    displayQt->setUseExactMatchPattern(state == Qt::Checked);
  });
  connect(fuzzySearchBox, &QCheckBox::stateChanged, displayQt, [this](int state) {
    displayQt->setUseFuzzyMatchPattern(state == Qt::Checked);
  });
  connect(wildcardSearchBox, &QCheckBox::stateChanged, displayQt, [this](int state) {
    displayQt->setUseWildcardPattern(state == Qt::Checked);
  });
  connect(subsearchSearchBox, &QCheckBox::stateChanged, displayQt, [this](int state) {
    displayQt->setUseSubsearchPattern(state == Qt::Checked);
  });

  connect(wildcardCharInput,
          &QLineEdit::textChanged,
          displayQt,
          [this, wildcardCharInput](const QString &text) {
            if (text.length() == 1) {
              displayQt->setWildcardChar(text[0].toLatin1());
            } else if (text.length() == 2) {
              const char current = displayQt->getWildcardChar();
              const std::string input = text.toStdString();
              if (input[0] == current) {
                wildcardCharInput->setText(QString(input[1]));
                displayQt->setWildcardChar(input[1]);
              } else if (input[1] == current) {
                wildcardCharInput->setText(QString(input[0]));
                displayQt->setWildcardChar(input[0]);
              }
            } else {
              wildcardCharInput->setText(QString(displayQt->getWildcardChar()));
            }
          });

  connect(subsearchLengthInput,
          QOverload<int>::of(&QSpinBox::valueChanged),
          displayQt,
          [this](int value) { displayQt->setMinSubsearchSize(value); });

  // Add widgets to the grid layout
  grid->addWidget(exactSearchBox, 0, 0);
  grid->addWidget(fuzzySearchBox, 1, 0);
  grid->addWidget(wildcardSearchBox, 2, 0);
  grid->addWidget(wildcardCharInput, 3, 1);
  grid->addWidget(subsearchSearchBox, 4, 0);
  grid->addWidget(subsearchLengthInput, 4, 1);

  QCheckBox *searchHidden = new QCheckBox("Search hidden Objects");
  QCheckBox *searchFolders = new QCheckBox("Search Folder Names");
  QCheckBox *searchFiles = new QCheckBox("Search File Names");
  QCheckBox *howToOpenResults = new QCheckBox("Open Folder beneath Result");

  searchHidden->setChecked(displayQt->searchHiddenObjects());
  searchFolders->setChecked(displayQt->isSetSearchFolderNames());
  searchFiles->setChecked(displayQt->isSetSearchFileNames());
  howToOpenResults->setChecked(displayQt->openFolderBeneath());

  connect(searchHidden, &QCheckBox::stateChanged, displayQt, [this](int state) {
    displayQt->setSearchHiddenObjects(state == Qt::Checked);
  });
  connect(searchFolders, &QCheckBox::stateChanged, displayQt, [this](int state) {
    displayQt->setSearchForFolderNames(state == Qt::Checked);
  });
  connect(searchFiles, &QCheckBox::stateChanged, displayQt, [this](int state) {
    displayQt->setSearchForFileNames(state == Qt::Checked);
  });
  connect(howToOpenResults, &QCheckBox::stateChanged, displayQt, [this](int state) {
    displayQt->setOpenFolderBeneath(state == Qt::Checked);
  });

  grid->addWidget(searchHidden, 5, 0);
  grid->addWidget(searchFolders, 6, 0);
  grid->addWidget(searchFiles, 7, 0);
  grid->addWidget(howToOpenResults, 8, 0);

  controlsGroup->setLayout(grid);
  return controlsGroup;
}

void FinderWidget::reset() { updateInfo("-", 0, "-"); }

void FinderWidget::updateInfo(const std::string &root_path,
                              const size_t num_files,
                              const std::string &indexing_date) {
  // Ensure this runs in the main thread
  if (QThread::currentThread() != this->thread()) {
    // Make copies of the arguments and invoke method in the main thread
    auto root_path_copy = root_path;
    auto indexing_date_copy = indexing_date;

    QMetaObject::invokeMethod(
        this,
        [this, root_path_copy, num_files, indexing_date_copy]() {
          updateInfo(root_path_copy, num_files, indexing_date_copy);
        },
        Qt::QueuedConnection);
    return;
  }

  // Update UI labels in the main thread
  rootPathLabel->setText(QString("Root Path: %1").arg(QString::fromStdString(root_path)));
  filesFoundLabel->setText(
      QString("Files Found: %1").arg(QString::fromStdString(std::to_string(num_files))));
  indexingDate->setText(indexing_date.c_str());
}
