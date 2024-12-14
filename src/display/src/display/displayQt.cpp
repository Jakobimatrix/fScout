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

  // Create a main layout to stack the top widget and the splitter
  QVBoxLayout *mainLayout = new QVBoxLayout();
  mainLayout->addWidget(splitter);

  // Set main layout to a central widget
  QWidget *centralWidget = new QWidget(this);
  centralWidget->setLayout(mainLayout);
  setCentralWidget(centralWidget);

  createActions();
  createMenus();
  createToolBars();
  createStatusBar();
  setUnifiedTitleAndToolBarOnMac(true);

  // set size, pos,scale like user has saved
  QPoint qpos(getDisplayPosX(), getDisplayPosY());
  QSize qsize(getDisplaySizeW(), getDisplaySizeH());
  resize(qsize);
  move(qpos);

  // Only scale the font. The window size was already set.
  changeScale(getDisplayScale(), true);

  const auto &path = Globals::getInstance().getAbsPath2Resources();
  QPixmap pixmap((path / "open.png").string().c_str());
  QIcon icon(pixmap.scaled(BASE_ICON_SIZE, BASE_ICON_SIZE, Qt::KeepAspectRatio));
  setWindowIcon(icon);
}

void DisplayQt::resetDisplayElements() {
  finder_widget->reset();
  finder_output_widget->reset();
}

void DisplayQt::updateInfo(const std::wstring &root_path,
                           const size_t num_files,
                           const std::wstring &indexingDate) {
  finder_widget->updateInfo(root_path, num_files, indexingDate);
}

void DisplayQt::onScaleChanged(const QString &scaleText) {
  bool ok;
  int scale = scaleText.leftRef(scaleText.length() - 1).toInt(&ok);  // Remove '%' and convert to int

  if (ok) {
    changeScale(scale, false);
    saveDisplayScale(scale);  // save new scale after setting it!
  }
}

void DisplayQt::changeScale(const int scale, const bool is_scale_on_load) {
  const double new_fraction = scale / 100.0;
  const double old_inverse_fraction = 100. / static_cast<double>(getDisplayScale());
  const double scale_fraction = new_fraction * old_inverse_fraction;

  const double fontScale = is_scale_on_load ? new_fraction : scale_fraction;

  QFont font = this->font();
  font.setPointSizeF(font.pointSizeF() * fontScale);
  setFont(font);
  // Scale the tooltip font size as well
  QFont tooltipFont = QToolTip::font();
  tooltipFont.setPointSizeF(tooltipFont.pointSizeF() * fontScale);
  QToolTip::setFont(tooltipFont);

  if (!is_scale_on_load) {  // on load resizes the windows already,  dont scale again.
    resize(this->size() * scale_fraction);
    finder_widget->resize(finder_widget->size() * scale_fraction);
    finder_output_widget->resize(finder_output_widget->size() * scale_fraction);
  }
  finder_output_widget->changeScale(fontScale);
}

const QStringList DisplayQt::getAvailableZoomLevels() {
  return {"100%", "125%", "150%", "175%", "200%"};
}

const int DisplayQt::getCurrentZoomLevelIndex(int currentZoom) {
  QStringList zoomLevels = getAvailableZoomLevels();
  QString zoomText = QString::number(currentZoom) + "%";

  int index = zoomLevels.indexOf(zoomText);
  return (index != -1) ? index : 0;  // If zoom level not found, default to 100% (index 0)
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

bool DisplayQt::askYesNoQuestion(const std::wstring &question, const std::wstring &title) {
  QMessageBox::StandardButton ret;
  ret = QMessageBox::question(this,
                              QString::fromStdWString(title),
                              QString::fromStdWString(question),
                              QMessageBox::Yes | QMessageBox::No);
  return ret == QMessageBox::Yes;
}

void DisplayQt::popup_info(const std::wstring &text, const std::wstring &title) {
  QMessageBox::information(
      this, QString::fromStdWString(text), QString::fromStdWString(title), QMessageBox::Ok);
}

void DisplayQt::popup_warning(const std::wstring &text, const std::wstring &title) {
  QMessageBox::warning(
      this, QString::fromStdWString(text), QString::fromStdWString(title), QMessageBox::Ok);
}

void DisplayQt::popup_error(const std::wstring &text, const std::wstring &title) {
  QMessageBox::critical(
      this, QString::fromStdWString(text), QString::fromStdWString(title), QMessageBox::Ok);
}

void DisplayQt::setWindowFilePath(const std::wstring &file_path) {
  QMainWindow::setWindowFilePath(QString::fromStdWString(file_path));
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

void DisplayQt::setStatus(const std::wstring &msg, int timeout) {
  statusBar()->showMessage(QString::fromStdWString(msg), timeout);
}

void DisplayQt::setStatus(const QString &msg, int timeout) {
  setStatus(msg.toStdWString(), timeout);
}

void DisplayQt::about() {

  const std::string info_text =
      "<b>Current Version:</b> " + Globals::getInstance().getVersion();
  QMessageBox::about(this, tr("About Finder"), tr(info_text.c_str()));
}

void DisplayQt::open() { Display::open(); }

void DisplayQt::visualize() { Display::visualize(); }

void DisplayQt::createActions() {
  const auto &path = Globals::getInstance().getAbsPath2Resources();

  // Helper function to create an action
  auto createAction = [this](const QString &iconPath,
                             const QString &actionText,
                             const QString &statusTip,
                             const QKeySequence &shortcut,
                             auto (DisplayQt::*slot)()) -> QAction * {
    QPixmap pixmap(iconPath);
    QIcon icon(pixmap.scaled(BASE_ICON_SIZE, BASE_ICON_SIZE, Qt::KeepAspectRatio));
    QAction *action = new QAction(icon, actionText, this);
    action->setStatusTip(statusTip);
    action->setShortcut(shortcut);
    connect(action, &QAction::triggered, this, slot);
    return action;
  };

  // Create actions
  const auto openPath = (path / "open.png").string();
  openAct = createAction(openPath.c_str(),
                         tr("&Open Folder..."),
                         tr("Open the base of your search."),
                         QKeySequence::Find,
                         &DisplayQt::open);


  const auto visualizePath = (path / "visualize.png").string();
  visualizeAct = createAction(visualizePath.c_str(),
                              tr("&Visualize Dictionary..."),
                              tr("Visualize the Indexing strategy."),
                              QKeySequence::Print,
                              &DisplayQt::visualize);


  const auto exitPathPathPathPath = (path / "exit.png").string();
  exitAct = createAction(exitPathPathPathPath.c_str(),
                         tr("&Exit"),
                         tr("Close the program."),
                         QKeySequence::Close,
                         &DisplayQt::close);

  aboutAct = new QAction(tr("About &Finder"), this);
  aboutAct->setStatusTip(tr("Show the Finders's About box."));
  connect(aboutAct, &QAction::triggered, this, &DisplayQt::about);
}

void DisplayQt::createMenus() {
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  menuBar()->addSeparator();

  editMenu = menuBar()->addMenu(tr("&Edit"));
#ifndef NDEBUG
  editMenu->addAction(visualizeAct);
#endif

  menuBar()->addSeparator();

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(aboutAct);
}

void DisplayQt::createToolBars() {
  fileToolBar = addToolBar(tr("File"));
  fileToolBar->addAction(openAct);
  fileToolBar->setIconSize(QSize(BASE_ICON_SIZE, BASE_ICON_SIZE));

  editToolBar = addToolBar(tr("Edit"));
#ifndef NDEBUG
  editToolBar->addAction(visualizeAct);
#endif
  // scale chooser
  QComboBox *scaleChooser = new QComboBox(this);
  scaleChooser->addItems(getAvailableZoomLevels());
  scaleChooser->setCurrentIndex(getCurrentZoomLevelIndex(getDisplayScale()));
  connect(scaleChooser, &QComboBox::currentTextChanged, this, &DisplayQt::onScaleChanged);
  editToolBar->addWidget(scaleChooser);
}

void DisplayQt::createStatusBar() {
  statusBar()->showMessage(tr("Ready"));
  QColor backGroundColor = palette().color(QPalette::Window);
  const std::string backgroud_style_sheet =
      "background-color: " + backGroundColor.name().toStdString() + ";";
  statusBar()->setStyleSheet(backgroud_style_sheet.c_str());
}


void DisplayQt::setSearchResults(const std::vector<std::filesystem::path> &searchResults,
                                 const std::wstring &search) {
  finder_output_widget->setSearchResults(searchResults, search);
}
