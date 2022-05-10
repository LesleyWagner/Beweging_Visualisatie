using HelixToolkit.Wpf;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace Beweging_Visualisatie_WPF
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private WifiModule wifi;
        private System.Timers.Timer timer_poll;
        private System.Timers.Timer timer_send;
        private LinkedList<Pose> poses;
        private LinkedList<Pose> newPoses;
        private WpfControlLibrary3.UserControl1 uc;
        private Vector3D rotation_axis;
        private Rotation3D init_rotation;
        private RotateTransform3D init_rotateTransform;
        private Rotation3D rotation_yaw;
        private Rotation3D rotation_pitch;
        private Rotation3D rotation_roll;
        private RotateTransform3D transform_yaw;
        private RotateTransform3D transform_pitch;
        private RotateTransform3D transform_roll;
        private Transform3DGroup f1car_transforms;
        private Model3DGroup f1car;
        private RotateTransform3D rotateTransform;
        private HelixViewport3D uc_viewport;
        private ModelVisual3D model3d;

        private ModelVisual3D device3D;

        //Path to the model file
        private const string MODEL_PATH = "Formula_1_mesh.obj";

        public MainWindow () {
            wifi = new WifiModule ();
            poses = new LinkedList<Pose> ();
            newPoses = new LinkedList<Pose> ();

            InitializeComponent ();
            device3D = new ModelVisual3D ();
            device3D.Content = import3D (MODEL_PATH);
            // Add to view port
            viewPort3d.Children.Add (device3D);
            rotation_axis = new Vector3D (1, 0, 0);

            viewPort3d.Camera.Position = new Point3D (0, 0, 1240);
            viewPort3d.Camera.LookDirection = new Vector3D (0, 0, -1);
            viewPort3d.Camera.UpDirection = new Vector3D (0, 1, 0);
        }

        private void Window_Loaded (object sender, RoutedEventArgs e) {
            //uc = new WpfControlLibrary3.UserControl1 ();
            
            //Grid uc_grid = (Grid)uc.Content;
            //uc_viewport = (HelixViewport3D)uc_grid.Children[0];
            //model3d = (ModelVisual3D)uc_viewport.Children[0];
            //f1car = (Model3DGroup)model3d.Content;
            //rotation_axis = new Vector3D (1, 0, 0);
            //rotation = new AxisAngleRotation3D (rotation_axis, 90);
            //rotateTransform = new RotateTransform3D (rotation);
            //init_rotation = new AxisAngleRotation3D (rotation_axis, 0);
            //init_rotateTransform = new RotateTransform3D (init_rotation);
            //f1car.Transform = init_rotateTransform;

            //Grid window_grid = (Grid)this.Content;
            //window_grid.Children.Add (uc);



            timer_poll = new System.Timers.Timer (30);
            timer_poll.Elapsed += pollData;
            timer_send = new System.Timers.Timer (1000);
            timer_send.Elapsed += sendData;
            timer_poll.AutoReset = false;
            timer_send.AutoReset = false;

            DispatcherTimer dispatcherTimer = new System.Windows.Threading.DispatcherTimer ();
            dispatcherTimer.Tick += timerUpdate_tick;
            dispatcherTimer.Interval = new TimeSpan (300000); // 30 ms interval
            dispatcherTimer.Start ();
        }

        private void ViewPort3d_CameraChanged (object sender, RoutedEventArgs e) {
            Debug.WriteLine ("Position: " + ((HelixViewport3D)e.Source).Camera.Position);
            Debug.WriteLine ("Direction: " + ((HelixViewport3D)e.Source).Camera.LookDirection);
            Debug.WriteLine ("Up direction: " + ((HelixViewport3D)e.Source).Camera.UpDirection);
        }

        private void timerUpdate_tick (object sender, EventArgs e) {
            if (wifi.hasData) {
                wifi.getData (ref newPoses);
                rotate_model (newPoses.Last.Value.orientation);

                newPoses.Clear ();
            }
        }

        private void pollData (Object sender, EventArgs e) {
            _ = wifi.updateData ();
            timer_poll.Enabled = true;
        }

        private void sendData (Object sender, EventArgs e) {
            _ = wifi.sendData ();
            timer_send.Enabled = true;
        }

        private void btn_connect_Click (object sender, RoutedEventArgs e) {
            wifi.connect ();
            timer_poll.Enabled = true;
            timer_send.Enabled = true;
        }

        private void btn_disconnect_Click (object sender, RoutedEventArgs e) {
            timer_poll.Enabled = false;
            timer_send.Enabled = false;
            wifi.disconnect ();
        }

        private void btn_reset_Click (object sender, RoutedEventArgs e) {
            // reset origin of the model
            viewPort3d.Camera.Transform = f1car_transforms;
        }

        private void rotate_model (Components orientation) {
            rotation_yaw = new AxisAngleRotation3D (new Vector3D (0, 1, 0), orientation.x);
            rotation_pitch = new AxisAngleRotation3D (new Vector3D (0, 0, 1), orientation.y);
            rotation_roll = new AxisAngleRotation3D (new Vector3D (1, 0, 0), orientation.z);
            transform_yaw = new RotateTransform3D (rotation_yaw);
            transform_pitch = new RotateTransform3D (rotation_pitch);
            transform_roll = new RotateTransform3D (rotation_roll);

            f1car_transforms = new Transform3DGroup ();
            f1car_transforms.Children.Add (transform_yaw);
            f1car_transforms.Children.Add (transform_pitch);
            f1car_transforms.Children.Add (transform_roll);

            device3D.Transform = f1car_transforms;
        }

        private void translate_model (Components position) {

        }

        /// <summary>
        /// Display 3D Model
        /// </summary>
        /// <param name="model">Path to the Model file</param>
        /// <returns>3D Model Content</returns>
        private Model3D import3D (string model) {
            Model3D device = null;
            try {
                //Adding a gesture here
                viewPort3d.RotateGesture = new MouseGesture (MouseAction.LeftClick);

                //Import 3D model file
                // ModelImporter import = new ModelImporter ();

                //Load the 3D model file
                // device = import.Load (MODEL_PATH);
                var r = new ObjReader (Dispatcher) { DefaultMaterial = Materials.Blue, Freeze = false };
                device = r.Read (MODEL_PATH);
            }
            catch (Exception e) {
                // Handle exception in case can not file 3D model
                MessageBox.Show ("Exception Error : " + e.StackTrace);
            }
            return device;
        }
    }
}
