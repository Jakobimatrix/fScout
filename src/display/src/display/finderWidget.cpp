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
  grid->setColumnStretch(0, 0);  // First column gets no flexible stretch
  grid->setColumnStretch(1, 0);  // Second column gets no extra stretch
  grid->setColumnStretch(2, 1);  // Third column gets flexible stretch

  constexpr int INPUT_WIDTH_EM = 50;

  // Checkboxes for search patterns
  QCheckBox *wildcardSearchBox = new QCheckBox("Wildcard (*) Search");
  QLineEdit *wildcardCharInput = new QLineEdit;
  wildcardCharInput->setMaxLength(2);
  wildcardCharInput->setMaximumWidth(INPUT_WIDTH_EM);
  wildcardCharInput->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  wildcardCharInput->setText(QString(displayQt->getWildcardChar()));

  constexpr int FUZZY_COEFF_STEP_SIZE = 10;

  QLabel *fuzzyTemperatureLabel = new QLabel("Fuzzy Search strength in %");
  QSpinBox *fuzzyTemperatureInput = new QSpinBox;
  QSlider *fuzzyTemperatureSlider = new QSlider(Qt::Horizontal);
  fuzzyTemperatureInput->setMinimum(static_cast<int>(Finder::MIN_FUZZY_COEFF * 100.f));
  fuzzyTemperatureInput->setMaximum(static_cast<int>(Finder::MAX_FUZZY_COEFF * 100.f));
  fuzzyTemperatureInput->setValue(static_cast<int>(displayQt->getFuzzyCoeff() * 100.f));
  fuzzyTemperatureInput->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  fuzzyTemperatureInput->setMaximumWidth(INPUT_WIDTH_EM);
  fuzzyTemperatureInput->setSingleStep(FUZZY_COEFF_STEP_SIZE);
  fuzzyTemperatureSlider->setMinimum(static_cast<int>(Finder::MIN_FUZZY_COEFF * 100.f));
  fuzzyTemperatureSlider->setMaximum(static_cast<int>(Finder::MAX_FUZZY_COEFF * 100.f));
  fuzzyTemperatureSlider->setValue(static_cast<int>(displayQt->getFuzzyCoeff() * 100.f));
  fuzzyTemperatureSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  fuzzyTemperatureSlider->setTickInterval(FUZZY_COEFF_STEP_SIZE);
  fuzzyTemperatureSlider->setSingleStep(FUZZY_COEFF_STEP_SIZE);


  wildcardSearchBox->setChecked(displayQt->usesWildcardPattern());

  // Connect signals to displayQt for search patterns
  connect(wildcardSearchBox, &QCheckBox::stateChanged, displayQt, [this](int state) {
    displayQt->setUseWildcardPattern(state == Qt::Checked);
    displayQt->searchAgain();
  });
  connect(fuzzyTemperatureInput,
          QOverload<int>::of(&QSpinBox::valueChanged),
          displayQt,
          [this](int value) {
            displayQt->setFuzzyCoeff(static_cast<float>(value) / 100.f);
            displayQt->searchAgain();
          });

  QObject::connect(fuzzyTemperatureSlider,
                   &QSlider::valueChanged,
                   fuzzyTemperatureInput,
                   [fuzzyTemperatureInput](int value) {
                     if (value % FUZZY_COEFF_STEP_SIZE != 0) {
                       return;
                     }
                     fuzzyTemperatureInput->setValue(value);
                   });

  QObject::connect(fuzzyTemperatureInput,
                   QOverload<int>::of(&QSpinBox::valueChanged),
                   fuzzyTemperatureSlider,
                   &QSlider::setValue);


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
            displayQt->searchAgain();
          });


  // Add widgets to the grid layout
  int row = 0;
  grid->addWidget(wildcardSearchBox, row, 0);
  grid->addWidget(wildcardCharInput, row++, 1);
  grid->addWidget(fuzzyTemperatureLabel, row, 0);
  grid->addWidget(fuzzyTemperatureInput, row, 1);
  grid->addWidget(fuzzyTemperatureSlider, row++, 2);

  QCheckBox *searchHidden = new QCheckBox("Search hidden Objects");
  QCheckBox *searchFolders = new QCheckBox("Search Folder Names");
  QCheckBox *searchFiles = new QCheckBox("Search File Names");
  QLabel *doubleClickDuration = new QLabel("Double Click Duration in MS");
  QSpinBox *doubleClickDurationInput = new QSpinBox;
  QSlider *fdoubleClickDurationSlider = new QSlider(Qt::Horizontal);
  doubleClickDurationInput->setMinimum(200);
  doubleClickDurationInput->setMaximum(999);
  doubleClickDurationInput->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  doubleClickDurationInput->setValue(displayQt->getDoubleClickInterval());
  doubleClickDurationInput->setMaximumWidth(INPUT_WIDTH_EM);
  fdoubleClickDurationSlider->setMinimum(200);
  fdoubleClickDurationSlider->setMaximum(999);
  fdoubleClickDurationSlider->setValue(displayQt->getDoubleClickInterval());
  fdoubleClickDurationSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  QObject::connect(fdoubleClickDurationSlider,
                   &QSlider::valueChanged,
                   doubleClickDurationInput,
                   &QSpinBox::setValue);
  QObject::connect(doubleClickDurationInput,
                   QOverload<int>::of(&QSpinBox::valueChanged),
                   fdoubleClickDurationSlider,
                   &QSlider::setValue);


  searchHidden->setChecked(displayQt->searchHiddenObjects());
  searchFolders->setChecked(displayQt->isSetSearchFolderNames());
  searchFiles->setChecked(displayQt->isSetSearchFileNames());

  connect(searchHidden, &QCheckBox::stateChanged, displayQt, [this](int state) {
    displayQt->setSearchHiddenObjects(state == Qt::Checked);
    displayQt->searchAgain();
  });
  connect(searchFolders, &QCheckBox::stateChanged, displayQt, [this](int state) {
    displayQt->setSearchForFolderNames(state == Qt::Checked);
    displayQt->searchAgain();
  });
  connect(searchFiles, &QCheckBox::stateChanged, displayQt, [this](int state) {
    displayQt->setSearchForFileNames(state == Qt::Checked);
    displayQt->searchAgain();
  });
  connect(doubleClickDurationInput,
          QOverload<int>::of(&QSpinBox::valueChanged),
          displayQt,
          [this](int value) { displayQt->setDoubleClickInterval(value); });

  grid->addWidget(searchHidden, row++, 0);
  grid->addWidget(searchFolders, row++, 0);
  grid->addWidget(searchFiles, row++, 0);
  grid->addWidget(doubleClickDuration, row, 0);
  grid->addWidget(doubleClickDurationInput, row, 1);
  grid->addWidget(fdoubleClickDurationSlider, row++, 2);

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
