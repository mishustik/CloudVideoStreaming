package httpserver;

import java.io.RandomAccessFile;

import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.channel.MessageEvent;
import org.jboss.netty.channel.SimpleChannelUpstreamHandler;
import org.jboss.netty.handler.codec.http.HttpChunk;
import org.jboss.netty.handler.codec.http.HttpResponse;

import android.os.Environment;

import com.example.cloudvideostreaming.MainActivity;

public class HttpSnoopClientHandler extends SimpleChannelUpstreamHandler {
	
	private boolean readingChunks;
		
	@Override
	public void messageReceived(ChannelHandlerContext ctx, MessageEvent e) throws Exception {
		
		RandomAccessFile f = new RandomAccessFile(Environment.getExternalStorageDirectory().getPath() + "/Download/Received_video.mp4","rws");
		//FileOutputStream f = new FileOutputStream(Environment.getExternalStorageDirectory().getPath() + "/Download/Received_video.mp4", true);
		//if(!f.exists()){
		//	f.createNewFile();
		//}
		
		//FileWriter fWriter = new FileWriter(f.getName(),true);
		//BufferedWriter bufferWriter = new BufferedWriter(fWriter);
		
		if (!readingChunks) {
			HttpResponse response = (HttpResponse) e.getMessage();
			
			System.out.println("STATUS: " + response.getStatus());
			System.out.println("VERSION: " + response.getProtocolVersion());
			System.out.println();
			
			if (!response.getHeaderNames().isEmpty()) {
				for (String name: response.getHeaderNames()) {
					for (String value: response.getHeaders(name)) {
						System.out.println("HEADER: " + name + " = " + value);
					}
				}
				System.out.println();
			}
			
			if (response.isChunked()) {
				readingChunks = true;
				System.out.println("Chunked content {");
			} else {
				ChannelBuffer content = response.getContent();
				if (content.readable()) {
					f.seek(MainActivity.shiftendfile);
					f.write(response.getContent().array());
					MainActivity.shiftendfile += response.getContent().array().length;
				}
			}
		} else {
			HttpChunk chunk = (HttpChunk) e.getMessage();
			if (chunk.isLast()) {
				readingChunks = false;
				System.out.println("} end of chunked content");
				f.close();
			} else {
			
			//	chunk.getContent().readBytes(f,10);
				f.seek(MainActivity.shiftendfile);
				f.write(chunk.getContent().array());
				MainActivity.shiftendfile += chunk.getContent().array().length;
				
			//	f.flush();
			//	System.out.print(chunk.getContent().toString(CharsetUtil.UTF_8));
			//	System.out.flush();
			}
		}
	}
}
