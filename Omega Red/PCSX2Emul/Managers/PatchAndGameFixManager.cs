/*  Omega Red - Client PS2 Emulator for PCs
*
*  Omega Red is free software: you can redistribute it and/or modify it under the terms
*  of the GNU Lesser General Public License as published by the Free Software Found-
*  ation, either version 3 of the License, or (at your option) any later version.
*
*  Omega Red is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
*  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*  PURPOSE.  See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with Omega Red.
*  If not, see <http://www.gnu.org/licenses/>.
*/

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;
using PCSX2Emul.Util;
using System.Reflection;
using System.IO.Compression;
using static PCSX2Emul.Tools.GameIndex;
using static PCSX2Emul.Util.Pcsx2Config.GamefixOptions;

namespace PCSX2Emul.Tools
{
        
    class PatchAndGameFixManager
    {
        private PatchAndGameFixManager() { }

        private static PatchAndGameFixManager m_Instance = null;

        public static PatchAndGameFixManager Instance { get { if (m_Instance == null) m_Instance = new PatchAndGameFixManager(); return m_Instance; } }
        
        public const string _UNKNOWN_GAME_KEY = "_UNKNOWN_GAME_KEY";

        public string curGameKey = _UNKNOWN_GAME_KEY;

        private void PatchesVerboseReset()
        {
	        curGameKey = _UNKNOWN_GAME_KEY;
        }

        public void LoadAllPatchesAndStuff()
        {
            PatchesVerboseReset();

            _ApplySettings();
        }
                
        private void sioSetGameSerial(string a_GameSerial)
        {
            PCSX2LibNative.Instance.setSioSetGameSerialFunc(a_GameSerial);
        }
        
        public void loadGameSettings(Pcsx2Config dest, string a_disc_serial)
        {
            var l_GameData = GameIndex.Instance.convert(a_disc_serial);

            loadGameSettings(dest, l_GameData);
        }


        string[] tbl_SpeedhackNames =
        {
        "mvuFlag",
        "InstantVU1"
        };

        string EnumToString(SpeedhackId id)
        {
            return tbl_SpeedhackNames[(int)id];
        }

        // Load Game Settings found in database
        // (game fixes, round modes, clamp modes, etc...)
        // Returns number of gamefixes set
        private int loadGameSettings(Pcsx2Config dest, GameIndex.GameData game)
        {
	        if( game == null ) return 0;

            int gf = 0;

            if (game.eeRoundMode != RoundMode.Undefined)
            {
                var value = (int)game.eeRoundMode;
                if (value >= 0)
                {
                    dest.Cpu.sseMXCSR.SetRoundMode(value);
                    gf++;
                }
            }

            if (game.vuRoundMode != RoundMode.Undefined)
            {
                var value = (int)game.vuRoundMode;
                if (value >= 0)
                {
                    dest.Cpu.sseVUMXCSR.SetRoundMode(value);
                    gf++;
                }
            }

            if (game.eeClampMode != ClampMode.Undefined)
            {
                int clampMode = (int)game.eeClampMode;

                dest.Cpu.Recompiler.fpuOverflow = (clampMode >= 1);
                dest.Cpu.Recompiler.fpuExtraOverflow = (clampMode >= 2);
                dest.Cpu.Recompiler.fpuFullMode = (clampMode >= 3);
                gf++;
            }

            if (game.vuClampMode != ClampMode.Undefined)
            {
                int clampMode = (int)game.vuClampMode;

                dest.Cpu.Recompiler.vuOverflow = (clampMode >= 1);
                dest.Cpu.Recompiler.vuExtraOverflow = (clampMode >= 2);
                dest.Cpu.Recompiler.vuSignOverflow = (clampMode >= 3);
                gf++;
            }

            // TODO - config - this could be simplified with maps instead of bitfields and enums
            for (SpeedhackId id = SpeedhackId.SpeedhackId_FIRST; id < SpeedhackId.SpeedhackId_COUNT; id++)
            {
                string key = String.Format("{0}SpeedHack", EnumToString(id));

                // Gamefixes are already guaranteed to be valid, any invalid ones are dropped
                if (game.speedHacks.ContainsKey(key))
                {
                    // Legacy note - speedhacks are setup in the GameDB as integer values, but
                    // are effectively booleans like the gamefixes
                    bool mode = game.speedHacks[key] != 0 ? true : false;
                    //dest.Speedhacks.Set(id, mode);

                    gf++;
                }
            }

            // TODO - config - this could be simplified with maps instead of bitfields and enums
            for (GamefixId id = GamefixId.GamefixId_FIRST; id < GamefixId.GamefixId_COUNT; id++)
            {
                string key = string.Format("{0}Hack", GameIndex.EnumToString(id));

                // Gamefixes are already guaranteed to be valid, any invalid ones are dropped
                if(game.gameFixes.Contains(key))
                {
                    // if the fix is present, it is said to be enabled
                    dest.Gamefixes.Set(id, true); 

                    gf++;

                    // The LUT is only used for 1 game so we allocate it only when the gamefix is enabled (save 4MB)
                    if (id == GamefixId.Fix_GoemonTlbMiss && true)
                        PCSX2LibNative.Instance.VTLB_Alloc_PpmapFinc();// vtlb_Alloc_Ppmap();
                }
            }

            return gf;
        }
                

        // This routine loads patches from the game database (but not the config/game fixes/hacks)
        // Returns number of patches loaded
        private uint loadPatchesFromGamesDB(string crc, GameIndex.GameData game)
        {
	        bool patchFound = false;

	        //string patch = "";

         //   if (game != null)
         //   {
         //       if (game.sectionExists("patches", crc)) {
         //           patch = game.getSection("patches", crc);
         //           patchFound = true;
         //       }
         //       else if (game.sectionExists("patches")) {
         //           patch = game.getSection("patches");
         //           patchFound = true;
         //       }
         //   }

         //   if (patchFound) TrimPatches(patch);

	        return 0;
        }

        private void TrimPatches(string s)
        {
            if(string.IsNullOrEmpty(s))
                return;

            var l_splits = s.Split(new char[]{'\n'});

            if(l_splits == null ||
                l_splits.Length <= 0)
                return;

            foreach (var patchString in l_splits)
            {
                if (patchString.StartsWith("/"))
                    continue;

                PCSX2LibNative.Instance.inifile_commandFunc(patchString.Replace("\r", ""));
            }
        }

        private void _ApplySettings()
        {
            //Threading::ScopedLock lock(mtx__ApplySettings);
	        // 'fixup' is the EmuConfig we're going to upload to the emulator, which very well may
	        // differ from the user-configured EmuConfig settings.  So we make a copy here and then
	        // we apply the commandline overrides and database gamefixes, and then upload 'fixup'
	        // to the global EmuConfig.
	        //
	        // Note: It's important that we apply the commandline overrides *before* database fixes.
	        // The database takes precedence (if enabled).

            //fixup = src;

            //const CommandlineOverrides& overrides( wxGetApp().Overrides );
            //if( overrides.DisableSpeedhacks || !g_Conf->EnableSpeedHacks )
            //    fixup.Speedhacks.DisableAll();

            //if( overrides.ApplyCustomGamefixes )
            //{
            //    for (GamefixId id=GamefixId_FIRST; id < pxEnumEnd; ++id)
            //        fixup.Gamefixes.Set( id, overrides.Gamefixes.Get(id) );
            //}
            //else if( !g_Conf->EnableGameFixes )
            //    fixup.Gamefixes.DisableAll();

            //if( overrides.ProfilingMode )
            //{
            //    fixup.GS.FrameLimitEnable = false;
            //    fixup.GS.VsyncEnable = VsyncMode::Off;
            //}

            var fixup = EmulInstance.InternalInstance.m_Pcsx2Config;

            string gameCRC = null;
            string gameSerial = null;
            string gamePatch = null;
            string gameFixes = null;
            string gameCheats = null;
            string gameWsHacks = null;

	        string gameName = null;
            string gameCompat = null;
            string gameMemCardFilter = null;

	        // The CRC can be known before the game actually starts (at the bios), so when
	        // we have the CRC but we're still at the bios and the settings are changed
	        // (e.g. the user presses TAB to speed up emulation), we don't want to apply the
	        // settings as if the game is already running (title, loadeding patches, etc).
	        bool ingame = (PCSX2LibNative.Instance.getElfCRCFunc() > 0 && (PCSX2LibNative.Instance.getGameLoadingFunc() || PCSX2LibNative.Instance.getGameStartedFunc()));

            gameCRC = EmulInstance.InternalInstance.ElfCRC.ToString("x");
            //if (ingame && !DiscSerial.IsEmpty()) gameSerial = L" [" + DiscSerial + L"]";

	        string newGameKey = ingame ? EmulInstance.InternalInstance.DiscSerial : PCSX2LibNative.Instance.getSysGetBiosDiscIDFunc();
	        bool verbose =  newGameKey != curGameKey && ingame;
	        //Console.WriteLn(L"------> patches verbose: %d   prev: '%s'   new: '%s'", (int)verbose, WX_STR(curGameKey), WX_STR(newGameKey));
            //SetupPatchesCon(verbose);

	        curGameKey = newGameKey;

            PCSX2LibNative.Instance.ForgetLoadedPatchesFunc();// ForgetLoadedPatches();

	        if (!string.IsNullOrEmpty(curGameKey))
	        {
                var l_GameData = GameIndex.Instance.convert(EmulInstance.InternalInstance.DiscSerial);

		        if (l_GameData != null)
		        {
                    //if (l_GameData.keyExists("MemCardFilter"))
                    //{
                    //    gameMemCardFilter = l_GameData.getString("MemCardFilter");
                    //}

                    if (fixup.EnablePatches)
                    {
                        loadPatchesFromGamesDB(gameCRC, l_GameData);

                        loadGameSettings(fixup, curGameKey);
                    }
		        }
	        }

	        if (!string.IsNullOrEmpty(gameMemCardFilter))
		        sioSetGameSerial(gameMemCardFilter);
	        else
		        sioSetGameSerial(curGameKey);

            if (string.IsNullOrEmpty(gameName) && string.IsNullOrEmpty(gameSerial) && string.IsNullOrEmpty(gameCRC))
	        {
		        // if all these conditions are met, it should mean that we're currently running BIOS code.
		        // Chances are the BiosChecksum value is still zero or out of date, however -- because
		        // the BIos isn't loaded until after initial calls to ApplySettings.

		        gameName = "Booting PS2 BIOS... ";
	        }
            
	        // wide screen patches
            if(!string.IsNullOrWhiteSpace(curGameKey))
                LoadPatches(gameCRC);
            
            gsUpdateFrequency(fixup);
        }

        public void LoadPatches(string gameCRC)
        {
            if (!EmulInstance.InternalInstance.DisableWideScreen && !string.IsNullOrEmpty(gameCRC))
            {
                var f = LoadPatchesFromZip(gameCRC);

                //if (int numberLoadedWideScreenPatches = LoadPatchesFromDir(gameCRC, GetCheatsWsFolder(), L"Widescreen hacks"))
                //{
                //    gameWsHacks.Printf(L" [%d widescreen hacks]", numberLoadedWideScreenPatches);
                //    Console.WriteLn(Color_Gray, "Found widescreen patches in the cheats_ws folder --> skipping cheats_ws.zip");
                //}
                //else
                //{
                //    // No ws cheat files found at the cheats_ws folder, try the ws cheats zip file.
                //    string cheats_ws_archive = Path::Combine(PathDefs::GetProgramDataDir(), wxFileName(L"cheats_ws.zip"));
                //    int numberDbfCheatsLoaded = LoadPatchesFromZip(gameCRC, cheats_ws_archive);
                //    PatchesCon->WriteLn(Color_Green, "(Wide Screen Cheats DB) Patches Loaded: %d", numberDbfCheatsLoaded);
                //    gameWsHacks.Printf(L" [%d widescreen hacks]", numberDbfCheatsLoaded);
                //}
            }
        }

        void gsUpdateFrequency(Pcsx2Config config)
        {
            //switch (g_LimiterMode)
            //{
            //case LimiterModeType::Limit_Nominal:
            //    config.GS.LimitScalar = g_Conf->Framerate.NominalScalar;
            //    break;
            //case LimiterModeType::Limit_Slomo:
            //    config.GS.LimitScalar = g_Conf->Framerate.SlomoScalar;
            //    break;
            //case LimiterModeType::Limit_Turbo:
            //    config.GS.LimitScalar = g_Conf->Framerate.TurboScalar;
            //    break;
            //default:
            //    pxAssert("Unknown framelimiter mode!");
            //}
            //UpdateVSyncRate();

            PCSX2LibNative.Instance.gsUpdateFrequencyCallFunc();
        }

        int LoadPatchesFromZip(string gameCRC)
        {
            int lresult = 0;
            
            Assembly l_assembly = Assembly.GetExecutingAssembly();

            Stream l_Cheats_WSStream = l_assembly.GetManifestResourceStream("PCSX2Emul.Assests.cheats_ws.zip");

            if (l_Cheats_WSStream != null && l_Cheats_WSStream.CanRead)
            {
                using (ZipArchive archive = new ZipArchive(l_Cheats_WSStream, ZipArchiveMode.Read))
                {

                    foreach (var item in archive.Entries)
                    {
                        if (item.Name.ToLower().Contains(gameCRC.ToLower()) && item.Name.ToLower().Contains(".pnach"))
                        {
                            StreamReader lg = new StreamReader(item.Open());

                            var lpatchesString = lg.ReadToEnd();

                            TrimPatches(lpatchesString);
                        }
                    }

                }

            }

            return lresult;
        }
    }
}
