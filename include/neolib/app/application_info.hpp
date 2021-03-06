// application_info.hpp
/*
 *  Copyright (c) 2007 Leigh Johnston.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of Leigh Johnston nor the names of any
 *       other contributors to this software may be used to endorse or
 *       promote products derived from this software without specific prior
 *       written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <neolib/neolib.hpp>
#include <boost/filesystem.hpp>
#include <neolib/core/string_ci.hpp>
#include <neolib/core/vector.hpp>
#include <neolib/core/string.hpp>
#include <neolib/app/version.hpp>
#include <neolib/app/i_application_info.hpp>

namespace neolib
{
    std::string settings_folder(const std::string& aApplicationName, const std::string& aCompanyName);

    typedef vector<string> program_arguments_t;
    inline program_arguments_t to_program_arguments(int argc, char* argv[])
    {
        program_arguments_t result;
        for (auto arg = 0; arg < argc; ++arg)
            result.push_back(neolib::string{ argv[arg] });
        return result;
    }

    class application_info : public i_application_info
    {
    public:
        application_info(
            const program_arguments_t& aArguments = program_arguments_t{},
            const std::string& aName = "<Program Name>",
            const std::string& aCompany = "<Company Name>",
            const neolib::version& aVersion = neolib::version{},
            const std::string& aCopyright = "<Copyright>",
            const std::string& aApplicationFolder = std::string{},
            const std::string& aSettingsFolder = std::string{},
            const std::string& aDataFolder = std::string{},
            const std::string& aPluginExtension = ".plg") :
            iArguments{ aArguments },
            iName{ aName },
            iCompany{ aCompany },
            iVersion{ aVersion },
            iCopyright{ aCopyright },
            iApplicationFolder{ aApplicationFolder.empty() ? boost::filesystem::current_path().generic_string() : aApplicationFolder },
            iSettingsFolder{ aSettingsFolder.empty() ? neolib::settings_folder(aName, aCompany) : aSettingsFolder },
            iDataFolder{ aDataFolder },
            iPluginExtension{ aPluginExtension }
        {
            if (std::find(std::next(iArguments.container().begin()), iArguments.container().end(), neolib::ci_string("/pocket")) != iArguments.container().end() ||
                std::find(std::next(iArguments.container().begin()), iArguments.container().end(), neolib::ci_string("-pocket")) != iArguments.container().end())
            {
                iSettingsFolder = iApplicationFolder;
            }
            if (iDataFolder.empty())
                iDataFolder = iSettingsFolder;
        }
        application_info(const i_application_info& aOther) :
            iArguments{ aOther.arguments() },
            iName{ aOther.name() },
            iCompany{ aOther.company() },
            iVersion{ aOther.version() },
            iCopyright{ aOther.copyright() },
            iApplicationFolder{ aOther.application_folder() },
            iSettingsFolder{ aOther.settings_folder() },
            iDataFolder{ aOther.data_folder() },
            iPluginExtension{ aOther.plugin_extension() }
        {
        }

    public:
        const i_vector<i_string>& arguments() const override { return iArguments; }
        const i_string& name() const override { return iName; }
        const i_string& company() const override { return iCompany; }
        const i_version& version() const override { return iVersion; }
        const i_string& copyright() const override { return iCopyright; }
        const i_string& application_folder() const override { return iApplicationFolder; }
        const i_string& settings_folder() const override { return iSettingsFolder; }
        const i_string& data_folder() const override { return iDataFolder; }
        const i_string& plugin_extension() const override { return iPluginExtension; }

    private:
        vector<string> iArguments;
        string iName;
        string iCompany;
        neolib::version iVersion;
        string iCopyright;
        string iApplicationFolder;
        string iSettingsFolder;
        string iDataFolder;
        string iPluginExtension;
    };
}
