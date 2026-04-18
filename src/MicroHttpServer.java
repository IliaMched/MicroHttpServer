import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.file.Files;
import java.nio.file.Path;

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

                Thread clientThread = new Thread(() -> handleClient(clientSocket));
                clientThread.start();
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
            String requestLine = reader.readLine();

            if (requestLine == null || requestLine.isEmpty()) {
                clientSocket.close();
                return;
            }

            System.out.println("----- REQUEST START -----");
            System.out.println("Thread: " + Thread.currentThread().getName());
            System.out.println(requestLine);

            String line;
            while ((line = reader.readLine()) != null && !line.isEmpty()) {
                System.out.println(line);
            }
            System.out.println("----- REQUEST END -----");

            String[] parts = requestLine.split(" ");

            if (parts.length != 3) {
                sendResponse(
                    output,
                    "400 Bad Request",
                    "text/html",
                    "<html><body><h1>400 Bad Request</h1></body></html>"
                );
                clientSocket.close();
                return;
            }

            String method = parts[0];
            String path = parts[1];
            String version = parts[2];

            System.out.println("Method: " + method);
            System.out.println("Path: " + path);
            System.out.println("Version: " + version);

            if (!method.equals("GET")) {
                sendResponse(
                    output,
                    "405 Method Not Allowed",
                    "text/html",
                    "<html><body><h1>405 Method Not Allowed</h1><p>Only GET is supported.</p></body></html>"
                );
            } else {
                if (path.equals("/")) {
                    path = "/index.html";
                } else if (path.equals("/about")) {
                    path = "/about.html";
                }

                Path filePath = Path.of("www", path.substring(1));

                if (Files.exists(filePath) && !Files.isDirectory(filePath)) {
                    String body = Files.readString(filePath);
                    sendResponse(output, "200 OK", "text/html", body);
                } else {
                    sendResponse(
                        output,
                        "404 Not Found",
                        "text/html",
                        "<html><body><h1>404 Not Found</h1><p>The page does not exist.</p><a href=\"/\">Home</a></body></html>"
                    );
                }
            }

            clientSocket.close();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void sendResponse(OutputStream output, String status, String contentType, String body) throws Exception {
        byte[] bodyBytes = body.getBytes();

        String response =
            "HTTP/1.1 " + status + "\r\n" +
            "Content-Type: " + contentType + "; charset=UTF-8\r\n" +
            "Content-Length: " + bodyBytes.length + "\r\n" +
            "Connection: close\r\n" +
            "\r\n";

        output.write(response.getBytes());
        output.write(bodyBytes);
        output.flush();
    }
} 