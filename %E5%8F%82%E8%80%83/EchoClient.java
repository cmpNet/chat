// 客户端程序 
import java.net.*;
import java.io.*;

public class EchoClient {
    public static void main(String[] args)throws Exception {
        if (args.length != 2) {
            System.out.println("用法：EchoClient <主机名> <端口号>");
            return ;
        }

        // 建立连接并打开相关联的输入流和输出流
        Socket socket = new Socket(args[0], Integer.parseInt(args[1]));
        System.out.println("当前socket信息：" + socket);
        PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
        BufferedReader in = new BufferedReader(new InputStreamReader
            (socket.getInputStream()));

        // 将控制台输入的字符串发送给服务端，并显示从服务端获取的处理结果
        BufferedReader stdIn = new BufferedReader(new InputStreamReader
            (System.in));
        String userInput;
        while ((userInput = stdIn.readLine()) != null) {
            out.println(userInput);
            System.out.println("返回：" + in.readLine());
        }
        stdIn.close();
        // 关闭连接
        out.close();
        in.close();
        socket.close();
    }
}
