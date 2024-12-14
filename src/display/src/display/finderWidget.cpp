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
  QGroupBox *infoGroup = new QGroupBox("Indexing Information", this);
  QFormLayout *formLayout = new QFormLayout(this);

  formLayout->addRow(new QLabel("Root Path:"), rootPathLabel);
  rootPathLabel->setWordWrap(true);
  formLayout->addRow(new QLabel("Files Found:"), filesFoundLabel);
  formLayout->addRow(new QLabel("Indexing Date:"), indexingDate);

  infoGroup->setLayout(formLayout);
  return infoGroup;
}

QGroupBox *FinderWidget::create_controlls() {
  QGroupBox *controlsGroup = new QGroupBox("Search Patterns");

  QGridLayout *grid = new QGridLayout;

  // Checkboxes for search patterns
  QCheckBox *wildcardSearchBox = new QCheckBox("Wildcard (*) Search");
  QLineEdit *wildcardCharInput = new QLineEdit;
  wildcardCharInput->setMaxLength(2);
  wildcardCharInput->setText(QString(displayQt->getWildcardChar()));

  QLabel *fuzzyTemperatureLabel = new QLabel("Fuzzy Search strength in %");
  QSpinBox *fuzzyTemperatureInput = new QSpinBox;
  fuzzyTemperatureInput->setMinimum(static_cast<int>(Finder::MIN_FUZZY_COEFF * 100.f));
  fuzzyTemperatureInput->setMaximum(static_cast<int>(Finder::MAX_FUZZY_COEFF * 100.f));
  fuzzyTemperatureInput->setValue(static_cast<int>(displayQt->getFuzzyCoeff() * 100.f));


  wildcardSearchBox->setChecked(displayQt->usesWildcardPattern());

  // Connect signals to displayQt for search patterns
  connect(wildcardSearchBox, &QCheckBox::stateChanged, displayQt, [this](int state) {
    displayQt->setUseWildcardPattern(state == Qt::Checked);
  });
  connect(fuzzyTemperatureInput,
          QOverload<int>::of(&QSpinBox::valueChanged),
          displayQt,
          [this](int value) {
            displayQt->setFuzzyCoeff(static_cast<float>(value) / 100.f);
          });

  connect(wildcardCharInput,
          &QLineEdit::textChanged,
          displayQt,
          [this, wildcardCharInput](const QString &text) {
            if (text.length() == 1) {
              const std::wstring input = text.toStdWString();
              displayQt->setWildcardChar(input[0]);
            } else if (text.length() == 2) {
              const wchar_t current = displayQt->getWildcardChar();
              const std::wstring input = text.toStdWString();
              if (input[0] == current) {
                wildcardCharInput->setText(QString(input[1]));
                displayQt->setWildcardChar(input[1]);
              } else if (input[1] == current) {
                wildcardCharInput->setText(QString(input[0]));
                displayQt->setWildcardChar(input[0]);
              }
            }
          });


  // Add widgets to the grid layout
  int row = 0;
  grid->addWidget(wildcardSearchBox, row, 0);
  grid->addWidget(wildcardCharInput, row++, 1);
  grid->addWidget(fuzzyTemperatureLabel, row, 0);
  grid->addWidget(fuzzyTemperatureInput, row++, 1);

  QCheckBox *searchHidden = new QCheckBox("Search hidden Objects");
  QCheckBox *searchFolders = new QCheckBox("Search Folder Names");
  QCheckBox *searchFiles = new QCheckBox("Search File Names");
  QLabel *doubleClickDuration = new QLabel("Double Click Duration in MS");
  QSpinBox *doubleClickDurationInput = new QSpinBox;
  doubleClickDurationInput->setMinimum(200);
  doubleClickDurationInput->setMaximum(1000);
  doubleClickDurationInput->setValue(displayQt->getDoubleClickInterval());


  searchHidden->setChecked(displayQt->searchHiddenObjects());
  searchFolders->setChecked(displayQt->isSetSearchFolderNames());
  searchFiles->setChecked(displayQt->isSetSearchFileNames());

  connect(searchHidden, &QCheckBox::stateChanged, displayQt, [this](int state) {
    displayQt->setSearchHiddenObjects(state == Qt::Checked);
  });
  connect(searchFolders, &QCheckBox::stateChanged, displayQt, [this](int state) {
    displayQt->setSearchForFolderNames(state == Qt::Checked);
  });
  connect(searchFiles, &QCheckBox::stateChanged, displayQt, [this](int state) {
    displayQt->setSearchForFileNames(state == Qt::Checked);
  });
  connect(doubleClickDurationInput,
          QOverload<int>::of(&QSpinBox::valueChanged),
          displayQt,
          [this](int value) { displayQt->setDoubleClickInterval(value); });

  grid->addWidget(searchHidden, row++, 0);
  grid->addWidget(searchFolders, row++, 0);
  grid->addWidget(searchFiles, row++, 0);
  grid->addWidget(doubleClickDuration, row, 0);
  grid->addWidget(doubleClickDurationInput, row++, 1);

  controlsGroup->setLayout(grid);
  return controlsGroup;
}

void FinderWidget::reset() { updateInfo(rootpath_default, 0, L"-"); }

void FinderWidget::updateInfo(const std::wstring &root_path,
                              const size_t num_files,
                              const std::wstring &indexing_date) {
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
  // Enable breaking on slashes using the "zero-width space" hint
  QString pathStr = QString::fromStdWString(root_path);
  pathStr.replace("/", "/\u200B");  // Insert zero-width space after each '/'
  rootPathLabel->setText(QString("Root Path: %1").arg(pathStr));
  // rootPathLabel->setText(QString("Root Path: %1").arg(QString::fromStdWString(root_path)));
  filesFoundLabel->setText(QString("Files Found: %1").arg(QString::number(num_files)));
  indexingDate->setText(QString::fromStdWString(indexing_date));
}
