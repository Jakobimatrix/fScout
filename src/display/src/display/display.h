#pragma once


#include <finder/Finder.h>

#include <array>
#include <filesystem>
#include <functional>
#include <settings.hpp>
#include <string>

// This parent class handels all the savings/loadings and other non visual related tasks
// thouse are handled by the respecting child class which uses a lib or whatever to actually
// display things. This allowes changeing the engine which displays without needing to rewrite
// code regarding the functions apart from displaying things.
class Display : public util::Settings {
 public:
  Display();

  virtual ~Display();

  using setSearchResult = std::function<void(const std::vector<std::string>&)>;

  // <SEARCH> functionallity is public for Bad Boy reinterpret_cast<come and arrestme> magic
  void search(const std::string&);
  void setUseExactMatchPattern(bool use) {
    finder.setUseExactMatchPattern(use);
  }
  void setUseCaseInsensitivePattern(bool use) {
    finder.setUseCaseInsensitivePattern(use);
  }
  void setUseFuzzyMatchPattern(bool use) {
    finder.setUseFuzzyMatchPattern(use);
  }
  void setUseWildcardPattern(bool use) { finder.setUseWildcardPattern(use); }
  char getWildcardChar() const { return finder.getWindcard(); }
  void setWildcardChar(const char wildchar) { finder.setWildcard(wildchar); }
  void setUseSubsearchPattern(const bool use) {
    finder.setUseSubsetPattern(use);
  }
  size_t getMinSubsearchSize() const {
    return finder.getMinSubPatternSearchSize();
  }
  void setMinSubsearchSize(const std::size_t size) {
    finder.setMinSubPatternSize(size);
  }

  bool usesExactPattern() const { return finder.usesExactPattern(); }
  bool usesCaseInsensitivePattern() const {
    return finder.usesCaseInsensitivePattern();
  }
  bool usesFuzzyMatchPattern() const { return finder.usesFuzzyMatchPattern(); }
  bool usesWildcardPattern() const { return finder.usesWildcardPattern(); }
  bool usesSubsearchPattern() const { return finder.usesSubsetPattern(); }
  void setSearchForFileNames(bool searchFileNames);
  void setSearchForFolderNames(bool searchFolderNames);
  bool isSetSearchFileNames() const { return finder.isSetSearchFileNames(); }
  bool isSetSearchFolderNames() const {
    return finder.isSetSearchFolderNames();
  }
  bool openFolderBeneath() const { return prefere_open_folder_beneath; }
  void setOpenFolderBeneath(const bool open_folder_beneath) {
    prefere_open_folder_beneath = open_folder_beneath;
  }
  // </SEARCH>


 protected:
  // user interaktions
  [[nodiscard]] bool save();
  void open();
  void loadOldIndex();

  /*!
   * \brief Implement a popup file Picker Dialog.
   * \param postfix The postfix of the file.
   * \return Returns the users choosen file and path.
   */
  virtual std::filesystem::path filePickerDialog(const std::string& postfix) = 0;

  /*!
   * \brief Implement a popup file Save Dialog.
   * \param postfix The postfix of the file.
   * \return Returns the users choosen file name and path.
   */
  virtual std::filesystem::path fileSaveDialog(const std::string& postfix) = 0;

  /*!
   * \brief Open a Dialog that lets the user choose 1 Folder.
   */
  virtual std::filesystem::path openDirChooserDialog() = 0;

  /*!
   * \brief Implement a popup displaying title and question with user yes no
   * option.
   * \param question The question displayed to the user.
   * \param title A title displayed over the question.
   * \return Returns the users decision.
   */
  virtual bool askYesNoQuestion(const std::string& question,
                                const std::string& title = "") = 0;

  /*!
   * \brief Implement a popup displaying title and information.
   * option.
   * \param text The information displayed to the user.
   * \param title A title displayed over the question.
   */
  virtual void popup_info(const std::string& text,
                          const std::string& title = "") = 0;

  /*!
   * \brief Implement a popup displaying title and warning.
   * option.
   * \param warning The warning displayed to the user.
   * \param title A title displayed over the question.
   */
  virtual void popup_warning(const std::string& warning,
                             const std::string& title = "") = 0;

  /*!
   * \brief Implement a popup displaying title and error.
   * option.
   * \param error The error message displayed to the user.
   * \param title A title displayed over the question.
   */
  virtual void popup_error(const std::string& error,
                           const std::string& title = "") = 0;


  /*!
   * \brief Display given string in head area of the window to
   * show which file is loaded.
   * \param file_path The path displayed to the user.
   */
  virtual void setWindowFilePath(const std::string& file_path) = 0;

  /*!
   * \brief Display status string in bottom area of the window to
   * show current information.
   * \param msg The information displayed to the user.
   * \param timeout_ms time in ms when the status shall disapear.
   * if timeout_ms = 0, display until other status is pushed.
   */
  virtual void setStatus(const std::string& msg, int timeout_ms = 0) = 0;

  /*!
   * \brief Display the search results
   * \param results The results of a search
   */
  virtual void setSearchResults(const std::vector<std::filesystem::path>& searchResults) = 0;

  /*!
   * \brief Reset Display to "uninitiated"
   */
  virtual void resetDisplayElements() = 0;

  /*!
   * \brief Update the displayed Info
   * \param root_path A string displaying the root of the search
   * \param saved If the user saved the indexing
   * \param num_files The number of files in the index
   * \param indexing_date The date of the indexing
   */
  virtual void updateInfo(const std::string& root_path,
                          const bool saved,
                          const size_t num_files,
                          const std::string& indexingDate) = 0;

  bool hasUnsavedChanges() const { return need_save; }

  [[nodiscard]] bool exitGracefully();

  const std::array<int, 4>& getDisplayProportions() const {
    return disp_pos_size;
  }
  int getDisplayPosX() const { return disp_pos_size[0]; }
  int getDisplayPosY() const { return disp_pos_size[1]; }
  int getDisplaySizeW() const { return disp_pos_size[2]; }
  int getDisplaySizeH() const { return disp_pos_size[3]; }

  void saveDisplayProperties(int x, int y, int w, int h) {
    disp_pos_size[0] = x;
    disp_pos_size[1] = y;
    disp_pos_size[2] = w;
    disp_pos_size[3] = h;
  }

  void saveDisplaySize(int w, int h) {
    disp_pos_size[2] = w;
    disp_pos_size[3] = h;
  }

  void saveDisplayPosition(int x, int y) {
    disp_pos_size[0] = x;
    disp_pos_size[1] = y;
  }

  void saveSplitWidgetState(const std::string& qt_split_widget_state) {
    this->split_widget_state = qt_split_widget_state;
  }

  const std::string& getSplitWidgetState() const { return split_widget_state; }

  void saveDisplayScale(int scale) { disp_scale = scale; }

  int getDisplayScale() const { return disp_scale; }

 private:
  bool ask4Save();

  void callbackIndexing(bool success, const std::string& msg);
  void callbackSearch(bool finnished, const std::vector<std::filesystem::path>& results);


  // calculation
  bool need_save = false;
  Finder finder;

  std::string last_search;


  // SETTINGS
  std::array<int, 4> disp_pos_size = {{50, 50, 600, 400}};
  const std::string DISP_POS_SIZE = "DisplayXYWH";
  std::string split_widget_state = "";
  const std::string SPLIT_WIDGET_STATE = "SplitWidgetState";
  int disp_scale = 100;
  const std::string DISP_SCALE = "DisplayScale";
  bool prefere_open_folder_beneath = false;
  const std::string PREFERE_OPEN_FOLDER_BENEATH = "OpenFolderBeneath";
};
