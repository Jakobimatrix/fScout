#pragma once


#include <finder/Finder.h>

#include <array>
#include <filesystem>
#include <functional>
#include <settings/settings.hpp>
#include <string>

// This parent class handels all the savings/loadings and other non visual related tasks
// thouse are handled by the respecting child class which uses a lib or whatever to actually
// display things. This allowes changeing the engine which displays without needing to rewrite
// code regarding the functions apart from displaying things.
using DisplaySettings = util::Settings<>;
class Display : public DisplaySettings {
 public:
  Display();

  virtual ~Display();

  using setSearchResult = std::function<void(const std::vector<std::string>&)>;

  // <SEARCH> functionallity is public for Bad Boy reinterpret_cast<come and arrestme> magic
  void search(const std::wstring&);
  void searchAgain();


  void setUseWildcardPattern(bool use) { finder.setUseWildcardPattern(use); }
  wchar_t getWildcardChar() const { return finder.getWindcard(); }
  void setWildcardChar(const wchar_t wildchar) { finder.setWildcard(wildchar); }

  bool usesWildcardPattern() const { return finder.usesWildcardPattern(); }
  void setSearchForFileNames(bool searchFileNames);
  void setSearchForFolderNames(bool searchFolderNames);
  bool isSetSearchFileNames() const { return finder.isSetSearchFileNames(); }
  bool isSetSearchFolderNames() const {
    return finder.isSetSearchFolderNames();
  }
  bool searchHiddenObjects() const { return finder.isSetSearchHiddenObjects(); }
  void setSearchHiddenObjects(const bool searchHidden) {
    finder.setSearchHiddenObjects(searchHidden);
  }
  float getFuzzyCoeff() const { return finder.getFuzzyCoefficient(); }
  void setFuzzyCoeff(const float coef) { finder.setFuzzyCoefficient(coef); }
  // </SEARCH>
  int getDoubleClickInterval() const { return doubleClickInterval_ms; }

  void setDoubleClickInterval(int interval_ms) {
    doubleClickInterval_ms = interval_ms;
  }


 protected:
  // user interaktions
  void open();
  void save();
  void visualize() const;
  void loadOldIndex();

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
  virtual bool askYesNoQuestion(const std::wstring& question, const std::wstring& title) = 0;

  /*!
   * \brief Implement a popup displaying title and information.
   * option.
   * \param text The information displayed to the user.
   * \param title A title displayed over the question.
   */
  virtual void popup_info(const std::wstring& text, const std::wstring& titel) = 0;

  /*!
   * \brief Implement a popup displaying title and warning.
   * option.
   * \param warning The warning displayed to the user.
   * \param title A title displayed over the question.
   */
  virtual void popup_warning(const std::wstring& warning, const std::wstring&) = 0;

  /*!
   * \brief Implement a popup displaying title and error.
   * option.
   * \param error The error message displayed to the user.
   * \param title A title displayed over the question.
   */
  virtual void popup_error(const std::wstring& error, const std::wstring& title) = 0;


  /*!
   * \brief Display given string in head area of the window to
   * show which file is loaded.
   * \param file_path The path displayed to the user.
   */
  virtual void setWindowFilePath(const std::wstring& file_path) = 0;

  /*!
   * \brief Display status string in bottom area of the window to
   * show current information.
   * \param msg The information displayed to the user.
   * \param timeout_ms time in ms when the status shall disapear.
   * if timeout_ms = 0, display until other status is pushed.
   */
  virtual void setStatus(const std::wstring& msg, int timeout_ms = 0) = 0;

  /*!
   * \brief Display the search results
   * \param results The results of a search
   * \param search The original search string
   */
  virtual void setSearchResults(const std::vector<std::filesystem::path>& searchResults,
                                const std::wstring& search) = 0;

  /*!
   * \brief Reset Display to "uninitiated"
   */
  virtual void resetDisplayElements() = 0;

  /*!
   * \brief Update the displayed Info
   * \param root_path A string displaying the root of the search
   * \param num_files The number of files in the index
   * \param indexing_date The date of the indexing
   */
  virtual void updateInfo(const std::wstring& root_path,
                          const size_t num_files,
                          const std::wstring& indexingDate) = 0;

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

  bool isReadyToSearch() const { return finder.isInitiated(); }

 private:
  void callbackIndexing(bool success, const std::wstring& msg);
  void callbackSearch(bool finnished,
                      const std::vector<std::filesystem::path>& results,
                      const std::wstring& search);


  // calculation
  Finder finder;

  std::wstring last_search;


  // SETTINGS
  std::array<int, 4> disp_pos_size        = {{50, 50, 600, 400}};
  const std::string DISP_POS_SIZE         = "DisplayXYWH";
  std::string split_widget_state          = "";
  const std::string SPLIT_WIDGET_STATE    = "SplitWidgetState";
  int disp_scale                          = 100;
  const std::string DISP_SCALE            = "DisplayScale";
  int doubleClickInterval_ms              = 250;
  const std::string DOUBLE_CLICK_INTERVAL = "DoubleClickInterval";
};
