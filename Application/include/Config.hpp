#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Log.hpp"
#include "ui/frame/Frame.hpp"
#include "ui/Theme.hpp"
#include "utils/nx/Button.hpp"

// Forward declarations as we only need the types here
enum class Language;
class minIni;

// The config class interacts with the config file stored on
// the SD Card to read/write both the application's and the
// sysmodule's configuration. Each option can be queried/set
// through this object.
class Config {
    private:
        // minIni interacts with .ini file
        minIni * ini;
        minIni * sysIni;

        // Cache of each key/value (read in on construction)
        int version_;

        std::string yandexMusicToken_;
        bool confirmClearQueue_;
        bool confirmExit_;
        Frame::Type initialFrame_;
        Log::Level logLevel_;
        bool skipWithLR_;

        Theme::Colour accentColour_;
        bool autoPlayerPalette_;
        Language language_;
        bool showTouchControls_;

        bool scanOnLaunch_;

        int searchMaxPlaylists_;
        int searchMaxArtists_;
        int searchMaxAlbums_;
        int searchMaxSongs_;

        bool autoLaunchService_;
        int setQueueMax_;
        int searchMaxPhrases_;
        int searchMaxScore_;

        // Read all values from app .ini
        void readConfig();

    public:
        // Takes path to config file, copying from romfs
        // if it doesn't exist
        Config(const std::string &);
        // Prepare object to interact with sysmodule config
        // Returns true if file exists and is successful, false otherwise
        bool prepareSys(const std::string &);

        // Version of the .ini (-1 by default)
        int version();
        bool setVersion(const int);

        // API Token for Yandex Music
        std::string yandexMusicToken();
        bool setYandexMusicToken(const std::string str);

        // Confirm clearing queue when playing a new song
        bool confirmClearQueue();
        bool setConfirmClearQueue(const bool);

        // Confirm exit
        bool confirmExit();
        bool setConfirmExit(const bool);

        // Frame to show on launch
        Frame::Type initialFrame();
        bool setInitialFrame(const Frame::Type);

        // Logging level
        Log::Level logLevel();
        bool setLogLevel(const Log::Level);

        // Whether to use L/R to skip
        bool skipWithLR();
        bool setSkipWithLR(const bool);

        // Accent colour
        Theme::Colour accentColour();
        bool setAccentColour(Theme::Colour);

        // Language
        Language language();
        bool setLanguage(const Language);

        // Auto player palette
        bool autoPlayerPalette();
        bool setAutoPlayerPalette(const bool);

        // Whether to show back/quit touch buttons
        bool showTouchControls();
        bool setShowTouchControls(const bool);

        // Scan library for changes on launch
        bool scanOnLaunch();
        bool setScanOnLaunch(const bool);

        // Limits for search result entries (-1 indicates no limit)
        int searchMaxPlaylists();
        bool setSearchMaxPlaylists(const int);
        int searchMaxArtists();
        bool setSearchMaxArtists(const int);
        int searchMaxAlbums();
        bool setSearchMaxAlbums(const int);
        int searchMaxSongs();
        bool setSearchMaxSongs(const int);

        // Whether to try to launch service automatically
        // if not running on launch
        bool autoLaunchService();
        bool setAutoLaunchService(const bool);

        // Maximum number of songs to set in queue when playing a new song
        int setQueueMax();
        bool setSetQueueMax(const int);

        // Minimum search score
        int searchMaxScore();
        bool setSearchMaxScore(const int);

        // Maximum search phrases
        int searchMaxPhrases();
        bool setSearchMaxPhrases(const int);

        // === Sysmodule Config === //
        // All methods start with sys*

        // Use key press combinations to adjust playback
        bool sysKeyComboEnabled();
        bool setSysKeyComboEnabled(const bool);

        // Key combinations
        std::vector<NX::Button> sysKeyComboNext();
        bool setSysKeyComboNext(const std::vector<NX::Button> &);
        std::vector<NX::Button> sysKeyComboPlay();
        bool setSysKeyComboPlay(const std::vector<NX::Button> &);
        std::vector<NX::Button> sysKeyComboPrev();
        bool setSysKeyComboPrev(const std::vector<NX::Button> &);

        // Pause when entering sleep
        bool sysPauseOnSleep();
        bool setSysPauseOnSleep(const bool);

        // Pause when headphones unplugged
        bool sysPauseOnUnplug();
        bool setSysPauseOnUnplug(const bool);

        // Sysmodule log level
        Log::Level sysLogLevel();
        bool setSysLogLevel(const Log::Level);

        // Seek accurately for MP3's (slow)
        bool sysMP3AccurateSeek();
        bool setSysMP3AccurateSeek(const bool);

        // MP3 Equalizer
        std::array<float, 32> sysMP3Equalizer();
        bool setSysMP3Equalizer(const std::array<float, 32> &);

        // Deletes minIni object
        ~Config();
};

#endif