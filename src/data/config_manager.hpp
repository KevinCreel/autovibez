// ConfigFile.h
// Class for reading named values from configuration files
// Richard J. Wagner  v2.1  24 May 2004  wagnerr@umich.edu

// Copyright (c) 2004 Richard J. Wagner
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

// Typical usage
// -------------
//
// Given a configuration file "settings.inp":
//   atoms  = 25
//   length = 8.0  # nanometers
//   name = Reece Surcher
//
// Named values are read in various ways, with or without default values:
//   ConfigFile config( "settings.inp" );
//   int atoms = config.read<int>( "atoms" );
//   double length = config.read( "length", 10.0 );
//   string author, title;
//   config.readInto( author, "name" );
//   config.readInto( title, "title", string("Untitled") );
//
// See file example.cpp for more examples.

#pragma once

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

using std::string;

namespace AutoVibez {
namespace Data {

class ConfigFile {
    // Data
protected:
    string myDelimiter;                   // separator between key and value
    string myComment;                     // separator between value and comments
    string mySentry;                      // optional string to signal end of file
    std::map<string, string> myContents;  // extracted keys and values

    typedef std::map<string, string>::iterator mapi;
    typedef std::map<string, string>::const_iterator mapci;

    // Methods
public:
    explicit ConfigFile(string filename, string delimiter = "=", string comment = "#", string sentry = "EndConfigFile");
    ConfigFile();

    // Search for key and read value or optional default value
    template <class T>
    T read(const string& key) const;  // call as read<T>
    template <class T>
    T read(const string& key, const T& value) const;
    template <class T>
    bool readInto(T& var, const string& key) const;
    template <class T>
    bool readInto(T& var, const string& key, const T& value) const;

    // Modify keys and values
    template <class T>
    void add(string key, const T& value);
    void remove(const string& key);

    // Check whether key exists in configuration
    bool keyExists(const string& key) const;

    // Check or change configuration syntax
    string getDelimiter() const {
        return myDelimiter;
    }
    string getComment() const {
        return myComment;
    }
    string getSentry() const {
        return mySentry;
    }
    string setDelimiter(const string& s) {
        string old = myDelimiter;
        myDelimiter = s;
        return old;
    }
    string setComment(const string& s) {
        string old = myComment;
        myComment = s;
        return old;
    }

    // Write or read configuration
    friend std::ostream& operator<<(std::ostream& os, const ConfigFile& cf);
    friend std::istream& operator>>(std::istream& is, ConfigFile& cf);

    std::string getPresetPath() const {
        return read<std::string>("preset_path", "");
    }
    std::string getTexturePath() const {
        return read<std::string>("texture_path", "");
    }
    int getAudioDeviceIndex() const {
        return read<int>("audio_device", 0);
    }
    bool getShowFps() const {
        return read<bool>("show_fps", false);
    }

    // Mix Management Settings
    std::string getYamlUrl() const {
        return read<std::string>("yaml_url", "");
    }
    std::string getMixesUrl() const {
        return read<std::string>("mixes_url", "");
    }
    bool getAutoDownload() const {
        return read<bool>("auto_download", true);
    }
    int getSeekIncrement() const {
        return read<int>("seek_increment", 60);  // 60 seconds default
    }
    int getVolumeStep() const {
        return read<int>("volume_step", 10);  // 10% default
    }

    // Crossfade settings
    bool getCrossfadeEnabled() const;
    int getCrossfadeDuration() const;

    // Auto-refresh settings
    int getRefreshInterval() const;

    // Genre settings
    std::string getPreferredGenre() const;

    std::string getFontPath() const {
        return read<std::string>("font_path", "");
    }

protected:
    template <class T>
    static string T_as_string(const T& t);
    template <class T>
    static T string_as_T(const string& s);
    static void trim(string& s);

    // Exception types
public:
    struct file_not_found {
        string filename;
        explicit file_not_found(const string& filename_ = string()) : filename(filename_) {}
    };
    struct key_not_found {  // thrown only by T read(key) variant of read()
        string key;
        explicit key_not_found(const string& key_ = string()) : key(key_) {}
    };
};

// Template method definitions must be inside the namespace

// static
// Convert from a T to a string
template <class T>
string ConfigFile::T_as_string(const T& t) {
    std::ostringstream oss;
    oss << t;
    return oss.str();
}

// static
// Convert from a string to a T
template <class T>
T ConfigFile::string_as_T(const string& s) {
    std::istringstream iss(s);
    T t;
    iss >> t;
    return t;
}

// static
// Convert from a string to a string (specialization)
template <>
inline string ConfigFile::string_as_T<string>(const string& s) {
    return s;
}

// static
// Convert from a string to a bool (specialization)
template <>
inline bool ConfigFile::string_as_T<bool>(const string& s) {
    bool b = true;
    string sup = s;
    for (auto& c : sup)
        c = toupper(c);
    if (sup == "FALSE" || sup == "F" || sup == "NO" || sup == "N" || sup == "0" || sup == "NONE")
        b = false;
    return b;
}

template <class T>
T ConfigFile::read(const string& key) const {
    mapci p = myContents.find(key);
    if (p == myContents.end())
        throw key_not_found(key);
    return string_as_T<T>(p->second);
}

template <class T>
T ConfigFile::read(const string& key, const T& value) const {
    mapci p = myContents.find(key);
    if (p == myContents.end())
        return value;
    return string_as_T<T>(p->second);
}

template <class T>
bool ConfigFile::readInto(T& var, const string& key) const {
    mapci p = myContents.find(key);
    bool found = (p != myContents.end());
    if (found)
        var = string_as_T<T>(p->second);
    return found;
}

template <class T>
bool ConfigFile::readInto(T& var, const string& key, const T& value) const {
    mapci p = myContents.find(key);
    bool found = (p != myContents.end());
    var = found ? string_as_T<T>(p->second) : value;
    return found;
}

template <class T>
void ConfigFile::add(string key, const T& value) {
    string v = T_as_string(value);
    trim(key);
    trim(v);
    myContents[key] = v;
}

}  // namespace Data
}  // namespace AutoVibez

// Release notes:
// v1.0  21 May 1999
//   + First release
//   + Template read() access only through non-member readConfigFile()
//   + ConfigurationFileBool is only built-in helper class
//
// v2.0  3 May 2002
//   + Shortened name from ConfigurationFile to ConfigFile
//   + Implemented template member functions
//   + Changed default comment separator from % to #
//   + Enabled reading of multiple-line values
//
// v2.1  24 May 2004
//   + Made template specializations inline to avoid compiler-dependent linkage
//   + Allowed comments within multiple-line values
//   + Enabled blank line termination for multiple-line values
//   + Added optional sentry to detect end of configuration file
//   + Rewrote messy trimWhitespace() function as elegant trim()
