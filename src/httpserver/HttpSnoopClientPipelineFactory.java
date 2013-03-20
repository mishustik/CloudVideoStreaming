package httpserver;

import static org.jboss.netty.channel.Channels.*;

import javax.net.ssl.SSLEngine;

import org.jboss.netty.channel.ChannelPipeline;
import org.jboss.netty.channel.ChannelPipelineFactory;
import org.jboss.netty.handler.codec.http.HttpClientCodec;
import org.jboss.netty.handler.codec.http.HttpContentDecompressor;
import org.jboss.netty.handler.ssl.SslHandler;

public class HttpSnoopClientPipelineFactory implements ChannelPipelineFactory{
	
	private final boolean ssl;
	
	public HttpSnoopClientPipelineFactory(boolean ssl) {
		
		this.ssl = ssl;
	}
	
	public ChannelPipeline getPipeline() throws Exception {
		
		ChannelPipeline pipeline = pipeline();
		
		if (ssl) {
			SSLEngine engine = SecureChatSslContextFactory.getClientContext().createSSLEngine();
			engine.setUseClientMode(true);
			
			pipeline.addLast("ssl", new SslHandler(engine));
		}
		
		pipeline.addLast("codec", new HttpClientCodec());
		
		pipeline.addLast("inflater", new HttpContentDecompressor());
		
		pipeline.addLast("handler", new HttpSnoopClientHandler());
		return pipeline;
	}
}
