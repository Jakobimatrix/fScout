#include "displayQt.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QtWidgets>
#include <globals/globals.hpp>
#include <globals/macros.hpp>

DisplayQt::DisplayQt() : Display() {
  finder_widget = new FinderWidget(this);
  finder_output_widget = new FinderOutputWidget(this);

  splitter = new QSplitter(this);
  splitter->addWidget(finder_widget);
  splitter->addWidget(finder_output_widget);
  splitter->setChildrenCollapsible(true);

  loadSplitterState();

  setCentralWidget(splitter);


  // Create a scale chooser
  QComboBox *scaleChooser = new QComboBox(this);
  scaleChooser->addItems({"100%", "125%", "150%", "175%", "200%"});
  scaleChooser->setCurrentIndex(0);  // Default to 100%

  connect(scaleChooser, &QComboBox::currentTextChanged, this, &DisplayQt::onScaleChanged);

  // Create a layout for the scale chooser
  QWidget *topWidget = new QWidget(this);
  QHBoxLayout *topLayout = new QHBoxLayout(topWidget);

  // Right align the scale chooser
  topLayout->addStretch();
  topLayout->addWidget(scaleChooser);

  // Create a main layout to stack the top widget and the splitter
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(topWidget);
  mainLayout->addWidget(splitter);

  // Set main layout to a central widget
  QWidget *centralWidget = new QWidget(this);
  centralWidget->setLayout(mainLayout);
  setCentralWidget(centralWidget);

  QPoint qpos(getDisplayPosX(), getDisplayPosY());
  QSize qsize(getDisplaySizeW(), getDisplaySizeH());
  resize(qsize);
  move(qpos);

  createActions();
  createMenus();
  createToolBars();
  createStatusBar();
  setUnifiedTitleAndToolBarOnMac(true);
}

void DisplayQt::resetDisplayElements() {
  finder_widget->reset();
  finder_output_widget->reset();
}

void DisplayQt::updateInfo(const std::string &root_path,
                           const bool saved,
                           const size_t num_files,
                           const std::string &indexingDate) {
  finder_widget->updateInfo(root_path, saved, num_files, indexingDate);
}

void DisplayQt::onScaleChanged(const QString& scaleText) {
    bool ok;
    int scale = scaleText.left(scaleText.length() - 1).toInt(&ok); // Remove '%' and convert to int

    if (ok) {
        QFont font = this->font();
        font.setPointSizeF(font.pointSizeF() * (scale / 100.0));
        setFont(font);

        // Adjust the entire window's scaling
        resize(this->size() * (scale / 100.0));
        finder_widget->resize(finder_widget->size() * (scale / 100.0));
        finder_output_widget->resize(finder_output_widget->size() * (scale / 100.0));
    }
}

void DisplayQt::close() { QMainWindow::close(); }

void DisplayQt::closeEvent(QCloseEvent *event) {
  if (!exitGracefully()) {
    event->ignore();
  }
  saveSplitterState();

  event->accept();
}


void DisplayQt::resizeEvent(QResizeEvent *event) {
  saveDisplaySize(event->size().width(), event->size().height());
  QMainWindow::resizeEvent(event);
}

void DisplayQt::moveEvent(QMoveEvent *event) {
  saveDisplayPosition(event->pos().x(), event->pos().y());
  QMainWindow::moveEvent(event);
}

bool DisplayQt::askYesNoQuestion(const std::string &question, const std::string &title) {
  QMessageBox::StandardButton ret;
  ret = QMessageBox::question(
      this, tr(title.c_str()), tr(question.c_str()), QMessageBox::Yes | QMessageBox::No);
  return ret == QMessageBox::Yes;
}

void DisplayQt::popup_info(const std::string &text, const std::string &title) {
  QMessageBox::information(this, tr(text.c_str()), tr(title.c_str()), QMessageBox::Ok);
}

void DisplayQt::popup_warning(const std::string &text, const std::string &title) {
  QMessageBox::warning(this, tr(text.c_str()), tr(title.c_str()), QMessageBox::Ok);
}

void DisplayQt::popup_error(const std::string &text, const std::string &title) {
  QMessageBox::critical(this, tr(text.c_str()), tr(title.c_str()), QMessageBox::Ok);
}

void DisplayQt::setWindowFilePath(const std::string &file_path) {
  const QString path(file_path.c_str());
  QMainWindow::setWindowFilePath(path);
}


std::filesystem::path DisplayQt::filePickerDialog(const std::string &filePostfix) {
  QString filter = QString("Files (*%1)").arg(QString::fromStdString(filePostfix));

  // Show the file dialog with the dynamically generated filter
  QString file_name = QFileDialog::getOpenFileName(this, tr("Open File"), "", filter);

  if (file_name.isEmpty()) {
    return std::filesystem::path();
  }

  return std::filesystem::path(file_name.toStdString());
}

std::filesystem::path DisplayQt::fileSaveDialog(const std::string &filePostfix) {
  QString filter = QString("Files (*%1)").arg(QString::fromStdString(filePostfix));

  QString file_name = QFileDialog::getSaveFileName(this, tr("Save File"), "", filter);
  if (file_name.isEmpty()) {
    return std::filesystem::path();
  }
  if (!file_name.endsWith(filePostfix.c_str())) {
    return std::filesystem::path(file_name.toStdString() + filePostfix);
  }
  return std::filesystem::path(file_name.toStdString());
}

std::filesystem::path DisplayQt::openDirChooserDialog() {

  QString dir = QFileDialog::getExistingDirectory(
      nullptr, "Select Folder", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  if (!dir.isEmpty()) {
    return std::filesystem::path(dir.toStdString());
  }

  return std::filesystem::path();
}

void DisplayQt::loadSplitterState() {
  if (!splitter) {
    return;
  }
  const std::string hex_string_state = getSplitWidgetState();

  constexpr size_t NUM_HEX_IN_BYTE = 2;
  constexpr size_t NUM_BITS_HEX = 4;

  if (hex_string_state.size() == 0 || hex_string_state.size() % NUM_HEX_IN_BYTE != 0) {
    return;
  }
  std::vector<char> bytes_state;
  bytes_state.reserve(hex_string_state.size() / NUM_HEX_IN_BYTE);

  for (size_t i = 0; i < hex_string_state.size(); i += NUM_HEX_IN_BYTE) {
    char byte = 0;
    for (size_t k = 0; k < NUM_HEX_IN_BYTE; ++k) {
      const char &c = hex_string_state[i + k];
      if (47 < c && c < 58) {
        byte |= (c - 48) << (k * NUM_BITS_HEX);
      } else if (64 < c && c < 71) {
        byte |= (c - 55) << (k * NUM_BITS_HEX);
      } else {
        ERROR(
            "Failed to restore QWidget Splitt Screen state. Non Hex value "
            "found.");
        return;
      }
    }
    bytes_state.push_back(byte);
  }

  const QByteArray qbytes_state = QByteArray::fromRawData(
      bytes_state.data(), static_cast<int>(bytes_state.size()));
  if (!splitter->restoreState(qbytes_state)) {
    ERROR("Failed to restore QWidget Splitt Screen state.");
  }
}

void DisplayQt::saveSplitterState() {
  if (!splitter) {
    return;
  }
  const QByteArray qbytes_state = splitter->saveState();
  std::string bit_string_state("");
  constexpr size_t NUM_BITS_HEX = 4;

  constexpr int FIRSTBITS_MASK = 0xF0;
  constexpr int SECONDBITS_MASK = 0x0F;
  for (auto c = qbytes_state.begin(); c != qbytes_state.end(); c++) {

    const int hex2 = (*c & FIRSTBITS_MASK) >> NUM_BITS_HEX;
    const int hex1 = *c & SECONDBITS_MASK;

    if (hex1 < 10) {
      const char hexc = static_cast<char>(hex1) + 48;  // 0-9
      bit_string_state += hexc;
    } else {
      const char hexc = static_cast<char>(hex1) + 55;  // A-F
      bit_string_state += hexc;
    }
    if (hex2 < 10) {
      const char hexc = static_cast<char>(hex2) + 48;  // 0-9
      bit_string_state += hexc;

    } else {
      const char hexc = static_cast<char>(hex2) + 55;  // A-F
      bit_string_state += hexc;
    }
  }

  saveSplitWidgetState(bit_string_state);
}

void DisplayQt::setStatus(const std::string &msg, int timeout) {
  statusBar()->showMessage(tr(msg.c_str()), timeout);
}

void DisplayQt::setStatus(const QString &msg, int timeout) {
  setStatus(msg.toStdString(), timeout);
}


bool DisplayQt::save() { return Display::save(); }


void DisplayQt::open() { Display::open(); }
void DisplayQt::load() { Display::loadOldIndex(); }

void DisplayQt::run() { WARNING("run does nothing"); }

void DisplayQt::about() {

  const std::string info_text =
      "<b>Current Version:</b> " + std::to_string(Globals::VERSION);
  QMessageBox::about(this, tr("About Finder"), tr(info_text.c_str()));
}


void DisplayQt::createActions() {
  const std::string path = Globals::getInstance().getAbsPath2Resources().string();

  // open
  openAct = new QAction(QIcon((path + "open.png").c_str()), tr("&Open Folder..."), this);
  openAct->setShortcuts(QKeySequence::Open);
  openAct->setStatusTip(tr("Open the base of your search."));
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

  // load
  loadAct = new QAction(QIcon((path + "load.png").c_str()), tr("&Load Index File..."), this);
  loadAct->setShortcuts(QKeySequence::Open);
  loadAct->setStatusTip(tr("Open an old indexed search."));
  connect(loadAct, SIGNAL(triggered()), this, SLOT(load()));

  // about
  aboutAct = new QAction(tr("About &Finder"), this);
  aboutAct->setStatusTip(tr("Show the Finders's About box."));
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  // save
  saveAct = new QAction(QIcon((path + "save.png").c_str()), tr("&Save Current Index"), this);
  saveAct->setStatusTip(tr("Save the current search tree."));
  connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

  // run
  runAct = new QAction(tr("start indexing"), this);
  runAct->setStatusTip(tr("Run the indexing"));
  connect(runAct, SIGNAL(triggered()), this, SLOT(run()));

  // exit
  exitAct = new QAction(tr("&Exit"), this);
  exitAct->setStatusTip(tr("Close the program."));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
}

void DisplayQt::createMenus() {
  fileMenu = menuBar()->addMenu(tr("&File"));
  if (fileMenu == nullptr) {
    ERROR("filemenue is nullptr :(");
    return;
  }
  fileMenu->addAction(openAct);
  fileMenu->addAction(loadAct);
  fileMenu->addAction(saveAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  editMenu = menuBar()->addMenu(tr("&Edit"));
  //  editMenu->addAction(cutAct);
  //  editMenu->addAction(copyAct);
  //  editMenu->addAction(pasteAct);

  runMenu = menuBar()->addMenu(tr("&Run"));
  runMenu->addAction(runAct);

  menuBar()->addSeparator();

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(aboutAct);
  helpMenu->addAction(aboutAct);
}

void DisplayQt::createToolBars() {
  fileToolBar = addToolBar(tr("File"));
  fileToolBar->addAction(openAct);
  fileToolBar->addAction(saveAct);

  editToolBar = addToolBar(tr("Edit"));
  //  editToolBar->addAction(cutAct);
  //  editToolBar->addAction(copyAct);
  //  editToolBar->addAction(pasteAct);

  runToolBar = addToolBar(tr("run"));
}

void DisplayQt::createStatusBar() {
  statusBar()->showMessage(tr("Ready"));
  QColor backGroundColor = palette().color(QPalette::Window);
  const std::string backgroud_style_sheet =
      "background-color: " + backGroundColor.name().toStdString() + ";";
  statusBar()->setStyleSheet(backgroud_style_sheet.c_str());
}


void DisplayQt::setSearchResults(const std::vector<std::filesystem::path> &searchResults) {
  finder_output_widget->setSearchResults(searchResults);
}
