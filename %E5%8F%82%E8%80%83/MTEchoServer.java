// 采用多线程方式实现的服务端程序 
import java.io.*;
import java.net.*;
public class MTEchoServer {
    public static void main(String[] args)throws IOException {
        if (args.length != 1) {
            System.out.println("用法：MTServer <端口号>");
            return ;
        }
        ServerSocket ss = new ServerSocket(Integer.parseInt(args[0]));
        System.out.println("服务程序正在监听端口：" + args[0]);
        for (;;)
            new EchoThread(ss.accept()).start();
    }
}

class EchoThread extends Thread {
    Socket socket;
    EchoThread(Socket s) {
        socket = s;
    }
    public void run() {
        System.out.println("正在为客户程序提供服务：" + socket);
        try {
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            String message;
            while ((message = in.readLine()) != null) {
                System.out.println(socket + "请求：" + message);
                out.println(message.toUpperCase());
            }
            out.close();
            in.close();
            socket.close();
        } catch (IOException exc) {
            exc.printStackTrace();
        }
    }
}
