import java.io.*;
import java.net.*;

class TCPServer {
    public static void main(String argv[]) throws Exception {
        String fromclient;
        String toclient;
		int port = 9999; //9999
		int clientsWaitingInQueue = 100;
		URL whatismyip = new URL("http://checkip.amazonaws.com"); 
		BufferedReader in = new BufferedReader(new InputStreamReader(
                whatismyip.openStream()));
		String ip_local = "192.168.0.101";
		String ip = in.readLine(); //you get the IP as a String
		InetAddress iAddr = InetAddress.getByName(ip_local);
		System.out.println("host: " + iAddr.toString() + " local: " + iAddr.isSiteLocalAddress());
		boolean accepted = false;
		ServerSocket Server = null;
		while (!accepted){
			try{
				Server = new ServerSocket(port, clientsWaitingInQueue, iAddr);
				accepted = true;
				System.out.println("port " + port + " accepted");
			}
			catch(Exception e){
				System.out.println("port incremented to " + (++port));
				break;
			}
		}
		System.out.println(Server.toString());

        System.out.println("TCPServer Waiting for client on port " + port);
		FileReader fr = new FileReader(new File("hello.html"));
		BufferedReader br = new BufferedReader(fr);
		String murphy = "", sCurrentLine;
		while ((sCurrentLine = br.readLine()) != null) {
				murphy += sCurrentLine + "\n";
		}
		System.out.println(murphy);
        while (true) {
            Socket connected = Server.accept();
            System.out.println(" THE CLIENT" + " " + connected.getInetAddress()
                    + ":" + connected.getPort() + " IS CONNECTED ");

            BufferedReader inFromUser = new BufferedReader(
                    new InputStreamReader(System.in));

            BufferedReader inFromClient = new BufferedReader(
                    new InputStreamReader(connected.getInputStream()));

            PrintWriter outToClient = new PrintWriter(
                    connected.getOutputStream(), true);

            while (true) {

                System.out.println("SEND(Type Q or q to Quit):");
                toclient = inFromUser.readLine();

                if (toclient.equals("q") || toclient.equals("Q")) {
                    outToClient.println(toclient);
                    connected.close();
                    break;
                } else {
                    outToClient.println(murphy);
                }
                fromclient = inFromClient.readLine();

                if (fromclient.equals("q") || fromclient.equals("Q")) {
                    connected.close();
                    break;
                } else {
                    System.out.println("RECIEVED:" + fromclient);
                }

            }

        }
    }
}
