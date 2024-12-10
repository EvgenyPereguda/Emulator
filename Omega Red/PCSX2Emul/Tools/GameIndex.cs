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
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using YamlDotNet.RepresentationModel;
using static PCSX2Emul.Util.Pcsx2Config.GamefixOptions;

namespace PCSX2Emul.Tools
{
    class GameIndex
    {
        static string[] tbl_GamefixNames =
        {
           "VuAddSub",
           "FpuCompare",
           "FpuMul",
           "FpuNegDiv",
           "XGKick",
           "IPUWait",
           "EETiming",
           "SkipMPEG",
           "OPHFlag",
           "DMABusy",
           "VIFFIFO",
           "VIF1Stall",
           "GIFFIFO",
           "GoemonTlb",
           "ScarfaceIbit",
           "CrashTagTeamRacingIbit",
           "VU0Kickstart"
        };

        public static string EnumToString(GamefixId id)
        {
            return tbl_GamefixNames[(int)id];
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

        public enum Compatibility
        {
            Unknown = 0,
            Nothing,
            Intro,
            Menu,
            InGame,
            Playable,
            Perfect
        };

        public enum RoundMode
        {
            Undefined = -1,
            Nearest = 0,
            NegativeInfinity,
            PositiveInfinity,
            ChopZero
        };

        public enum ClampMode
        {
            Undefined = -1,
            Disabled = 0,
            Normal,
            Extra,
            Full
        };
        
        public class Patches
        {
            private string m_patches = "";

            public void addPatch(string a_patch)
            {
                m_patches += a_patch;
            }

            public string patches { get { return m_patches; } }
        }

        public class GameData
        {                       
            public struct Patch
            {
                public string author;
                public List<string> patchLines;
            };

            public bool isValid = true;

            public string name;
            public string region;
            public Compatibility compat = Compatibility.Unknown;
            public RoundMode eeRoundMode = RoundMode.Undefined;
            public RoundMode vuRoundMode = RoundMode.Undefined;
            public ClampMode eeClampMode = ClampMode.Undefined;
            public ClampMode vuClampMode = ClampMode.Undefined;
            public List<string> gameFixes = new List<string>();
            public Dictionary<string, int> speedHacks = new Dictionary<string, int>();
            public List<string> memcardFilters = new List<string>();
            public Dictionary<string, Patch> patches = new Dictionary<string, Patch>();

        }

        private Dictionary<string, GameData> m_gameCollection = new Dictionary<string,GameData>();

        private static GameIndex m_Instance = null;

        public static GameIndex Instance { get { if (m_Instance == null) m_Instance = new GameIndex(); return m_Instance; } }

        private GameIndex()
        {
            loadGameIndex();
        }
        
        private void loadGameIndex()
        {           

            Assembly l_assembly = Assembly.GetExecutingAssembly();

            Stream l_GameIndexStream = l_assembly.GetManifestResourceStream("PCSX2Emul.Assests.GameIndex.yaml");
            
            if (l_GameIndexStream != null && l_GameIndexStream.CanRead)
            {
                // Load the stream
                var yaml = new YamlStream();

                yaml.Load(new StreamReader(l_GameIndexStream));

                // Examine the stream
                var mapping =
                    (YamlMappingNode)yaml.Documents[0].RootNode;

                foreach (var entry in mapping.Children)
                {
                    // we don't want to throw away the entire GameDB file if a single entry is made incorrectly,
                    // but we do want to yell about it so it can be corrected
                    try
                    {
                        // Serials and CRCs must be inserted as lower-case, as that is how they are retrieved
                        // this is because the application may pass a lowercase CRC or serial along
                        //
                        // However, YAML's keys are as expected case-sensitive, so we have to explicitly do our own duplicate checking

                        var serial = entry.Key.ToString();

                        if(m_gameCollection.ContainsKey(serial))
                            continue;

                        var value = entryFromYaml(serial, entry.Value as YamlMappingNode);
                        
                        m_gameCollection.Add(serial, value);
                    }
                    catch(Exception)
                    {

                    }
                }
            }
        }
        
        public GameData convert(string a_disc_serial)
        {
            GameData l_result = null;

            do
            {
                if(m_gameCollection.ContainsKey(a_disc_serial))
                {
                    l_result = m_gameCollection[a_disc_serial];
                }
                
            } while (false);

            return l_result;
        }

        

        List<string> convertMultiLineStringToVector(string multiLineString)
        {
            List<string> lines = new List<string>();

            StringReader stringReader = new StringReader(multiLineString);
            
	        while (true)
	        {
                string line = stringReader.ReadLine();

                if (string.IsNullOrEmpty(line))
                    break;

                lines.Add(line);
	        }

	        return lines;
        }

        GameData entryFromYaml(string serial, YamlMappingNode node)
        {
            GameData gameEntry = new GameData();
	        try
	        {
                if (node == null)
                    throw new Exception();
                               
                foreach (var item in node.Children)
                {
                    var value = item.Value.ToString();

                    switch (item.Key.ToString())
                    {
                        case "name":
                            gameEntry.name = value;
                            break;
                        case "region":
                            gameEntry.region = value;
                            break;
                        case "compat":
                            Enum.TryParse(value, out gameEntry.compat);
                            break;
                        case "roundModes":
                            // Safely grab round mode and clamp modes from the YAML, otherwise use defaults
                            if (item.Value.NodeType == YamlNodeType.Mapping)
                            {
                                foreach (var mode in (item.Value as YamlMappingNode).Children)
                                {
                                    if(mode.Key.ToString() == "eeRoundMode")
                                        Enum.TryParse(mode.Value.ToString(), out gameEntry.eeRoundMode);

                                    if (mode.Key.ToString() == "vuRoundMode")
                                        Enum.TryParse(mode.Value.ToString(), out gameEntry.vuRoundMode);
                                }
                            }
                            break;
                        case "clampModes":
                            // Safely grab round mode and clamp modes from the YAML, otherwise use defaults
                            if (item.Value.NodeType == YamlNodeType.Mapping)
                            {
                                foreach (var mode in (item.Value as YamlMappingNode).Children)
                                {
                                    if (mode.Key.ToString() == "eeClampMode")
                                        Enum.TryParse(mode.Value.ToString(), out gameEntry.eeClampMode);

                                    if (mode.Key.ToString() == "vuClampMode")
                                        Enum.TryParse(mode.Value.ToString(), out gameEntry.vuClampMode);
                                }
                            }
                            break;
                        case "gameFixes":
                            if(item.Value.NodeType == YamlNodeType.Sequence)
                            foreach (var fix in (item.Value as YamlSequenceNode).Children)
                            {
                                bool fixValidated = false;
                                for (GamefixId id = GamefixId.GamefixId_FIRST; id < GamefixId.GamefixId_COUNT; id++)
                                {
                                    string validFix = string.Format("{0}Hack", EnumToString(id));
                                    if (validFix == fix.ToString())
                                    {
                                        fixValidated = true;
                                        break;
                                    }
                                }
                                if (fixValidated)
                                {
                                    gameEntry.gameFixes.Add(fix.ToString());
                                }
                            }
                            break;
                        case "speedHacks":
                            // Validate speed hacks, invalid ones will be dropped!
                            if (item.Value.NodeType == YamlNodeType.Mapping)
                                foreach (var entry in (item.Value as YamlMappingNode).Children)
                                {
                                    string speedHack = entry.Key.ToString();
                                    bool speedHackValidated = false;
                                    for (SpeedhackId id = SpeedhackId.SpeedhackId_FIRST; id < SpeedhackId.SpeedhackId_COUNT; id++)
                                    {
                                        string validSpeedHack = string.Format("{0}SpeedHack", EnumToString(id));
                                        
                                        if (validSpeedHack == speedHack)
                                        {
                                            speedHackValidated = true;
                                            break;
                                        }
                                    }
                                    if (speedHackValidated)
                                    {
                                        int intValue = 0;

                                        if(int.TryParse(entry.Value.ToString(), out intValue))
                                        {
                                            gameEntry.speedHacks.Add(speedHack, intValue);
                                        }
                                    }
                                }
                            break;
                        case "memcardFilters":
                            if (item.Value.NodeType == YamlNodeType.Sequence)
                                foreach (var entry in (item.Value as YamlSequenceNode).Children)
                                {
                                    gameEntry.memcardFilters.Add(entry.ToString());
                                }
                            break;
                        case "patches":
                            // Validate speed hacks, invalid ones will be dropped!
                            if (item.Value.NodeType == YamlNodeType.Mapping)
                                foreach (var entry in (item.Value as YamlMappingNode).Children)
                                {
                                    string crc = entry.Key.ToString();
                                    if (gameEntry.patches.ContainsKey(crc))
                                    {
                                        continue;
                                    }
                                    var patchNode = entry.Value;

                                    GameData.Patch patchCol;

                                    patchCol.author = "";
                                    patchCol.patchLines = convertMultiLineStringToVector(patchNode["content"].ToString());
                                    gameEntry.patches.Add(crc, patchCol);
                                }                    
                            break;
                        default:
                            break;
                    }

                }
            }
	        catch (Exception)
	        {
		        gameEntry.isValid = false;
	        }
	        return gameEntry;
        }
    }
}
