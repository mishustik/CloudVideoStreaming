package httpserver;

import java.io.File;
import java.net.InetSocketAddress;
import java.net.URI;
import java.util.concurrent.Executors;

import org.jboss.netty.bootstrap.ClientBootstrap;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelFuture;
import org.jboss.netty.channel.socket.nio.NioClientSocketChannelFactory;
import org.jboss.netty.handler.codec.http.CookieEncoder;
import org.jboss.netty.handler.codec.http.DefaultHttpRequest;
import org.jboss.netty.handler.codec.http.HttpHeaders;
import org.jboss.netty.handler.codec.http.HttpMethod;
import org.jboss.netty.handler.codec.http.HttpRequest;
import org.jboss.netty.handler.codec.http.HttpVersion;

import android.os.Environment;

import com.example.cloudvideostreaming.MainActivity;

public class HttpSnoopClient {

	private final URI uri;
		
	public HttpSnoopClient(URI uri) {
		this.uri = uri;
	}
	
	public Boolean run() {
		String scheme = uri.getScheme() == null? "http" : uri.getScheme();
		String host = uri.getHost() == null? "localhost" : uri.getHost();
		int port = uri.getPort();
		if (port == -1) {
			if (scheme.equalsIgnoreCase("http")) {
				port = 80;
			} else if (scheme.equalsIgnoreCase("https")) {
				port = 443;
			}
		}
		
		if (!scheme.equalsIgnoreCase("http") && !scheme.equalsIgnoreCase("https")) {
			System.err.println("Only HTTP(S) is supported.");
			return false;
		}
		
		boolean ssl = scheme.equalsIgnoreCase("https");
		
		ClientBootstrap bootstrap = new ClientBootstrap(
				new NioClientSocketChannelFactory(
						Executors.newCachedThreadPool(),
						Executors.newCachedThreadPool()));
		
		bootstrap.setPipelineFactory(new HttpSnoopClientPipelineFactory(ssl));
		
		ChannelFuture future = bootstrap.connect(new InetSocketAddress(host, port));
		
		Channel channel = future.awaitUninterruptibly().getChannel();
		if(!future.isSuccess()) {
			future.getCause().printStackTrace();
			bootstrap.releaseExternalResources();
			return false;
		}
		
		HttpRequest request = new DefaultHttpRequest(
				HttpVersion.HTTP_1_1, HttpMethod.GET, uri.getRawPath());
		request.setHeader(HttpHeaders.Names.HOST, host);
		request.setHeader(HttpHeaders.Names.CONNECTION, HttpHeaders.Values.CLOSE);
		request.setHeader(HttpHeaders.Names.ACCEPT_ENCODING, HttpHeaders.Values.GZIP);
		
		CookieEncoder httpCookieEncoder = new CookieEncoder(false);
		httpCookieEncoder.addCookie("my-cookie","foo");
		httpCookieEncoder.addCookie("another-cookie","bar");
		request.setHeader(HttpHeaders.Names.COOKIE, httpCookieEncoder.encode());
		
		channel.write(request);
		
		channel.getCloseFuture().awaitUninterruptibly();
		
		bootstrap.releaseExternalResources();
		
		if (MainActivity.DecodingFlag == 1){
			MainActivity.ReceivedFlag = 1;
		}
		
		return true;
	}
	
	public static void main(String[] args) throws Exception {
		
		int ViCo = 1;
		String SubS;
		Boolean Flag = true;
		File file = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS) + args[0].substring(37,52) + ViCo + 
				args[0].substring(53,args[0].length()));
		
		if(args.length != 1) {
			System.err.println("Usage: " + HttpSnoopClient.class.getSimpleName() + " <URL>");
			return;
		}
		
		SubS = args[0];
		while (ViCo <= MainActivity.NumofLastDecFile){
			if((Flag) && (file.exists())){
				URI uri = new URI(SubS);
				Flag = new HttpSnoopClient(uri).run();
				//if (ViCo == 9)
				//	Flag = false;
				SubS = args[0].substring(0,52) + ViCo + args[0].substring(53,args[0].length());
				ViCo++;
				System.out.println(SubS);
				file = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS) + args[0].substring(37,52) + ViCo + args[0].substring(53,args[0].length()));
			}
		}
		URI uri = new URI(SubS);
		Flag = new HttpSnoopClient(uri).run();
		MainActivity.DecodingFlag = 1;
	}
}
