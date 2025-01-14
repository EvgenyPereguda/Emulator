﻿/*  Omega Red - Client PS2 Emulator for PCs
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

using Omega_Red.Managers;
using Omega_Red.Tools;
using Omega_Red.Tools.Converters;
using Omega_Red.Panels;
using Omega_Red.Util;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;
using Omega_Red.Properties;
using Omega_Red.Emulators;

namespace Omega_Red
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {


        public MainWindow()
        {
            SaveStateManager.Instance.init();

            TexturePackControl.Instance.init();

            Capture.TargetTexture.Instance.init();

            InitializeComponent();

            LockScreenManager.Instance.show();

            ConfigManager.Instance.SwitchControlModeEvent += Instance_SwitchControlModeEvent;

            Emul.Instance.ChangeStatusEvent += Instance_ChangeStatusEvent;

            MediaRecorderManager.Instance.ShowWarningEvent += Instance_ShowWarningEvent;

            PadControlManager.Instance.ShowWarningEvent += Instance_ShowWarningEvent;

            MediaRecorderManager.Instance.ShowInfoEvent += Instance_ShowInfoEvent;

            BiosManager.Instance.ShowErrorEvent += Instance_ShowErrorEvent;

            BiosControl.ShowErrorEvent += Instance_ShowErrorEvent;

//#if DEBUG

            WindowState = System.Windows.WindowState.Normal;

            WindowStyle = System.Windows.WindowStyle.SingleBorderWindow;
//#endif
        }

        private void Instance_ShowWarningEvent(string a_message)
        {
            mTaskbarIcon.ShowBalloonTip(Title, a_message, Hardcodet.Wpf.TaskbarNotification.BalloonIcon.Warning);
        }

        private void Instance_ShowInfoEvent(string a_message)
        {
            mTaskbarIcon.ShowBalloonTip(Title, a_message, Hardcodet.Wpf.TaskbarNotification.BalloonIcon.Info);
        }

        private void Instance_ShowErrorEvent(string a_message)
        {
            mTaskbarIcon.ShowBalloonTip(Title, a_message, Hardcodet.Wpf.TaskbarNotification.BalloonIcon.Error);
        }

        void Instance_ChangeStatusEvent(Emul.StatusEnum a_Status)
        {
            if (!mButtonControl && a_Status == Emul.StatusEnum.Started)
            {
                mMediaCloseBtn_Click(null, null);

                mControlCloseBtn_Click(null, null);
            }
        }

        private bool mButtonControl = false;

        void Instance_SwitchControlModeEvent(bool obj)
        {
            var l_LeftWidthConverter = Resources["mControlLeftWidthOffset"] as WidthConverter;

            var l_RightWidthConverter = Resources["mControlRightWidthOffset"] as WidthConverter;

            mButtonControl = obj;

            var l_TouchDragBtnWidth = (double)App.Current.Resources["TouchDragBtnWidth"];

            if (obj)
            {
                if (l_LeftWidthConverter != null)
                    l_LeftWidthConverter.Offset = -l_TouchDragBtnWidth - 2.5;

                if (l_RightWidthConverter != null)
                    l_RightWidthConverter.Offset = 0.0;
            }
            else
            {

                if (l_LeftWidthConverter != null)
                    l_LeftWidthConverter.Offset = -l_TouchDragBtnWidth - 10;

                if (l_RightWidthConverter != null)
                    l_RightWidthConverter.Offset = -l_TouchDragBtnWidth - 10;
            }
        }

        public void loadModules()
        {
            string l_warning = "";

            do
            {

                if (Settings.Default.GoogleAccountIsChecked)
                    SocialNetworks.Google.GoogleAccountManager.Instance.tryAuthorize();


                if (!RTMPNative.Instance.isInit)
                {
                    l_warning = "RTMP is not nitialized!!!";

                    break;
                }
                                                
                Application.Current.Dispatcher.BeginInvoke(DispatcherPriority.Background, (ThreadStart)delegate ()
                {

                    if (m_PadPanel.Content != null && m_PadPanel.Content is DisplayControl)
                    {
                        Emulators.Emul.Instance.setVideoPanel((m_PadPanel.Content as DisplayControl).VideoPanel);
                        
                        ScreenshotsManager.Instance.setVideoPanel((m_PadPanel.Content as DisplayControl).VideoPanel);

                        MediaSourcesManager.Instance.DisplayControl = m_PadPanel.Content as DisplayControl;
                    }

                    var wih = new System.Windows.Interop.WindowInteropHelper(App.Current.MainWindow);

                    App.CurrentWindowHandler = wih.Handle;

                    MediaSourcesManager.Instance.load(()=>
                    {
                        SocialNetworks.Google.GoogleAccountManager.Instance.sendEvent();

                        Emul.Instance.updateInitilize();

                        LockScreenManager.Instance.hide();
                    });

                    IsoManager.Instance.load();
                });
                
                Capture.MediaStream.Instance.setConnectionFunc(RTMPNative.Instance.Connect);

                Capture.MediaStream.Instance.setDisconnectFunc(RTMPNative.Instance.Disconnect);

                Capture.MediaStream.Instance.setWriteFunc(RTMPNative.Instance.Write);

                Capture.MediaStream.Instance.setIsConnectedFunc(RTMPNative.Instance.IsConnected);

            } while (false);

            if(!string.IsNullOrWhiteSpace(l_warning))
            {
                LockScreenManager.Instance.hide();

                Application.Current.Dispatcher.BeginInvoke(DispatcherPriority.Background, (ThreadStart)delegate ()
                {
                    Instance_ShowWarningEvent(l_warning);
                });
            }
        }
        
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            ThreadStart loadModulesStart = new ThreadStart(loadModules);

            Thread loadModulesThread = new Thread(loadModulesStart);

            loadModulesThread.Start();
        }

        public void rebindControlPanel()
        {
            Binding binding = new Binding();
            binding.Source = mControlGrid;
            binding.Path = new PropertyPath(FrameworkElement.ActualWidthProperty);
            binding.Converter = new WidthConverter() { Offset = -5.0, Scale = -1.0 };
            mControlGrid.SetBinding(Canvas.LeftProperty, binding);
        }

        public void rebindMediaPanel()
        {
            Binding binding = new Binding();
            binding.Source = mMediaGrid;
            binding.Path = new PropertyPath(FrameworkElement.ActualWidthProperty);
            binding.Converter = new WidthConverter() { Offset = -5.0, Scale = -1.0 };
            mMediaGrid.SetBinding(Canvas.RightProperty, binding);
        }

        private void mControlCloseBtn_Click(object sender, RoutedEventArgs e)
        {
            int timestamp = new TimeSpan(DateTime.Now.Ticks).Milliseconds;

            MouseButton l_mouseButton = MouseButton.Left;

            MouseButtonEventArgs l_UncheckedEvent = new MouseButtonEventArgs(Mouse.PrimaryDevice, timestamp, l_mouseButton);

            l_UncheckedEvent.RoutedEvent = CheckBox.UncheckedEvent;

            l_UncheckedEvent.Source = mControlChkBtn;

            mControlChkBtn.RaiseEvent(l_UncheckedEvent);

            mControlChkBtn.IsChecked = false;
        }

        private void Storyboard_Completed(object sender, EventArgs e)
        {
            if (!mButtonControl)
            {
                rebindControlPanel();
            }
            else
            {
                var l_TouchDragBtnWidth = (double)App.Current.Resources["TouchDragBtnWidth"];

                Binding binding = new Binding();
                binding.Source = mControlGrid;
                binding.Path = new PropertyPath(FrameworkElement.ActualWidthProperty);
                binding.Converter = new WidthConverter() { Offset = l_TouchDragBtnWidth - 2.0, Scale = -1.0 };
                mControlGrid.SetBinding(Canvas.LeftProperty, binding);
            }
        }

        private void mMediaCloseBtn_Click(object sender, RoutedEventArgs e)
        {
            int timestamp = new TimeSpan(DateTime.Now.Ticks).Milliseconds;

            MouseButton l_mouseButton = MouseButton.Left;

            MouseButtonEventArgs l_UncheckedEvent = new MouseButtonEventArgs(Mouse.PrimaryDevice, timestamp, l_mouseButton);

            l_UncheckedEvent.RoutedEvent = CheckBox.UncheckedEvent;

            l_UncheckedEvent.Source = mControlChkBtn;

            mMediaChkBtn.RaiseEvent(l_UncheckedEvent);

            mMediaChkBtn.IsChecked = false;
        }

        private void Storyboard_Completed_1(object sender, EventArgs e)
        {
            if (!mButtonControl)
            {
                rebindMediaPanel();
            }
        }

        private void Window_StateChanged(object sender, EventArgs e)
        {
            if (WindowState == System.Windows.WindowState.Maximized)
                WindowStyle = System.Windows.WindowStyle.None;
            else
                WindowStyle = System.Windows.WindowStyle.SingleBorderWindow;
        }
        
        public Grid getMediaGrid()
        {
            return mMediaGrid;
        }

        public MediaPanel getMediaPanel()
        {
            return mMediaPanel;
        }

        public ControlPanel getControlPanel()
        {
            return mControlPanel;
        }

        public Grid getControlGrid()
        {
            return mControlGrid;
        }

        public CheckBox getControlChkBtn()
        {
            return mControlChkBtn;
        }

        public CheckBox getMediaChkBtn()
        {
            return mMediaChkBtn;
        }
               
        private void Window_Closed(object sender, EventArgs e)
        {
            Settings.Default.Save();

            mTaskbarIcon.Dispose();
        }
    }
}
