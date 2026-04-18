import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;

public class MicroHttpServer {

    public static void main(String[] args) {
        int port = 8080;

        System.out.println("Starting server on port " + port + "...");

        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("Server is running.");
            System.out.println("Go to http://localhost:8080");

            while (true) {
                Socket clientSocket = serverSocket.accept();
                System.out.println("\nClient connected: " + clientSocket.getInetAddress());

                handleClient(clientSocket);
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void handleClient(Socket clientSocket) {
        try (
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(clientSocket.getInputStream())
            );
            OutputStream output = clientSocket.getOutputStream()
        ) {
            System.out.println("----- REQUEST START -----");

            String line;
            while ((line = reader.readLine()) != null && !line.isEmpty()) {
                System.out.println(line);
            }

            System.out.println("----- REQUEST END -----");

            String body =
                "<html><body>" +
                "<h1>It works</h1>" +
                "<p>Your server is running.</p>" +
                "</body></html>";

            String response =
                "HTTP/1.1 200 OK\r\n" +
                "Content-Type: text/html\r\n" +
                "Content-Length: " + body.getBytes().length + "\r\n" +
                "\r\n" +
                body;

            output.write(response.getBytes());
            output.flush();

            clientSocket.close();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}