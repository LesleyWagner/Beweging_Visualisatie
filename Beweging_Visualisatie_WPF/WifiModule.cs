using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Beweging_Visualisatie_WPF
{
    class WifiModule
    {
        private const string hostname = "ESP8266";
        private const string ipAddress = "10.10.10.1";
        private const int port = 4210;
        private List<Components> positions = new List<Components> (); // position buffer
        private List<Components> orientations = new List<Components> (); // orientations buffer
        private const int packetSize = 33;
        private const string sendMessage = "connected";

        private UdpClient udpClient;
        private bool connected = false;

        [StructLayout (LayoutKind.Explicit)]
        struct byte_array
        {
            [FieldOffset (0)]
            public byte byte1;

            [FieldOffset (1)]
            public byte byte2;

            [FieldOffset (2)]
            public byte byte3;

            [FieldOffset (3)]
            public byte byte4;

            [FieldOffset (0)]
            public float float1;
        }

        public WifiModule () {
            udpClient = new UdpClient ();
        }

        /*
         * Connect to UDP server with IP address 10.10.10.1 on port 4210.
        */
        public void connect () {
            try {
                udpClient.Connect (IPAddress.Parse (ipAddress), port);
                connected = true;
            }
            catch (Exception e) {
                Debug.Print (e.Message);
            }

            //while (!udpClient.Client.Poll(1000, SelectMode.SelectRead)) {
            //    udpClient.Connect(IPAddress.Parse("10.10.10.1"), port);
            //};
            //Debug.Print("Connection established.");
            //if (udpClient.Client.Poll(1000, SelectMode.SelectRead))
            //{
            //    Debug.Print("Client is readable");
            //}
            //else
            //{
            //    Debug.Print("Data can't be read.");
            //}
        }

        /*
         * Disconnect from UDP server with IP address 10.10.10.1 on port 4210.
        */
        public void disconnect () {
            try {
                connected = false;
                udpClient.Close ();
            }
            catch (Exception e) {
                Debug.Print (e.Message);
            }
        }

        /*
         * Indicates whether new pose data has been received.
        */
        public bool hasData { get; private set; }

        /*
         * Gets new pose data (orientation and position). Adds new data to the end of the poses list.
         * Doesn't modify the pose list in any other way. Indicates that the data has been retrieved by
         * setting the 'newData' field to false. Empties the position and orientation buffers.
         * @param poses - A linked list of poses that stores the latest orientations and positions of the car object.
         * @postconditions - Modifies the 'poses' list by adding new data to the end.
        */
        public void getData (ref LinkedList<Pose> poses) {
            for (int i = 0; i < orientations.Count; i++) {
                poses.AddLast (new Pose () { position = positions[i], orientation = orientations[i] });
            }
            positions.Clear ();
            orientations.Clear ();
            hasData = false;
        }

        /*
         * Sends data to the ESP8266. Returns the number of bytes sent.
        */
        public int sendData () {
            return udpClient.Send (System.Text.Encoding.UTF8.GetBytes (sendMessage.ToCharArray ()), sendMessage.Length);
        }

        /*
         * Polls received sensor data from the ESP8266.
        */
        public bool updateData () {
            if (connected) {
                try {
                    // Creates an IPEndPoint to record the IP Address and port number of the sender.
                    IPEndPoint remoteIpEndPoint = new IPEndPoint (IPAddress.Any, 0);
                    byte[] data;
                    int resultIndex;
                    Debug.WriteLine ("There are " + udpClient.Available + " bytes available.");
                    while (udpClient.Available >= packetSize) {
                        data = udpClient.Receive (ref remoteIpEndPoint);
                        resultIndex = 0;
                        // synchronize data with synchronization byte '_'
                        while (data.Length - resultIndex >= packetSize) {
                            if ((char)data[resultIndex] == '_') {
                                resultIndex++;
                                // position data
                                Components position = new Components ();
                                // Debug.WriteLine((char)data[resultIndex]);
                                if ((char)data[resultIndex] == 'P') {
                                    resultIndex++;
                                    if ((char)data[resultIndex] == 'x') {
                                        resultIndex++;
                                        // Debug.Print(data[resultIndex].ToString());
                                        byte_array posx;
                                        posx.float1 = 0;
                                        posx.byte1 = data[resultIndex++];
                                        posx.byte2 = data[resultIndex++];
                                        posx.byte3 = data[resultIndex++];
                                        posx.byte4 = data[resultIndex++];
                                        position.x = posx.float1;
                                        // Debug.Print("x: " + position.x.ToString());
                                    }
                                    if ((char)data[resultIndex] == 'y') {
                                        resultIndex++;
                                        byte_array posy;
                                        posy.float1 = 0;
                                        posy.byte1 = data[resultIndex++];
                                        posy.byte2 = data[resultIndex++];
                                        posy.byte3 = data[resultIndex++];
                                        posy.byte4 = data[resultIndex++];
                                        position.y = posy.float1;
                                        // Debug.Print("y: " + position.y.ToString());
                                    }
                                    if ((char)data[resultIndex] == 'z') {
                                        resultIndex++;
                                        byte_array posz;
                                        posz.float1 = 0;
                                        posz.byte1 = data[resultIndex++];
                                        posz.byte2 = data[resultIndex++];
                                        posz.byte3 = data[resultIndex++];
                                        posz.byte4 = data[resultIndex++];
                                        position.z = posz.float1;
                                        // Debug.Print("z: " + position.z.ToString());
                                    }
                                    positions.Add (position);
                                }
                                // orientation data
                                Components orientation = new Components ();
                                if ((char)data[resultIndex] == 'O') {
                                    resultIndex++;
                                    if ((char)data[resultIndex] == 'x') {
                                        resultIndex++;
                                        byte_array orix;
                                        orix.float1 = 0;
                                        orix.byte1 = data[resultIndex++];
                                        orix.byte2 = data[resultIndex++];
                                        orix.byte3 = data[resultIndex++];
                                        orix.byte4 = data[resultIndex++];
                                        orientation.x = orix.float1;
                                    }
                                    if ((char)data[resultIndex] == 'y') {
                                        resultIndex++;
                                        byte_array oriy;
                                        oriy.float1 = 0;
                                        oriy.byte1 = data[resultIndex++];
                                        oriy.byte2 = data[resultIndex++];
                                        oriy.byte3 = data[resultIndex++];
                                        oriy.byte4 = data[resultIndex++];
                                        orientation.y = oriy.float1;
                                    }
                                    if ((char)data[resultIndex] == 'z') {
                                        resultIndex++;
                                        byte_array oriz;
                                        oriz.float1 = 0;
                                        oriz.byte1 = data[resultIndex++];
                                        oriz.byte2 = data[resultIndex++];
                                        oriz.byte3 = data[resultIndex++];
                                        oriz.byte4 = data[resultIndex++];
                                        orientation.z = oriz.float1;
                                    }
                                    orientations.Add (orientation);
                                }
                                hasData = true;
                            }
                            else {
                                resultIndex++;
                            }
                        }
                    }
                    return true;
                }
                catch (Exception) {
                    Debug.WriteLine ("Here the exception happens.");
                    return false;
                }
            }
            else {
                return false;
            }
        }
    }
}
