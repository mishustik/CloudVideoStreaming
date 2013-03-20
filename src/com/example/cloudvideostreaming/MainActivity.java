package com.example.cloudvideostreaming;

//import java.io.File;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.Date;
import java.util.List;
import java.util.Random;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import org.jboss.netty.bootstrap.Bootstrap;
import org.jboss.netty.bootstrap.ClientBootstrap;
import org.jboss.netty.bootstrap.ServerBootstrap;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelEvent;
import org.jboss.netty.channel.ChannelFactory;
import org.jboss.netty.channel.ChannelFuture;
import org.jboss.netty.channel.ChannelFutureListener;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.channel.ChannelPipeline;
import org.jboss.netty.channel.ChannelPipelineFactory;
import org.jboss.netty.channel.Channels;
import org.jboss.netty.channel.DownstreamMessageEvent;
import org.jboss.netty.channel.MessageEvent;
import org.jboss.netty.channel.SimpleChannelHandler;
import org.jboss.netty.channel.socket.nio.NioClientSocketChannelFactory;
import org.jboss.netty.channel.socket.nio.NioServerSocketChannelFactory;
import org.jboss.netty.handler.codec.serialization.ClassResolvers;
import org.jboss.netty.handler.codec.serialization.ObjectDecoder;
import org.jboss.netty.handler.codec.serialization.ObjectEncoder;
import org.libsdl.app.SDLActivity;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.DownloadManager;
import android.app.DownloadManager.Query;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.database.Cursor;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.Menu;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;

@TargetApi(11)
public class MainActivity extends Activity {
	private static native int VideoDecoding(String filename, String filename2);
	private static native int EndCheck();
			
	SurfaceView mPreview;
	SurfaceHolder holder;
	MediaPlayer mediaPlayer;
	private DownloadManager manager;
	private long downloadReference;
	HttpClientServer test1 = new HttpClientServer();	
	public static int shiftendfile = 0;
	public static int NumofLastDecFile = 1000000;
	public static int DecodingFlag = 0;
	public static int ReceivedFlag = 0;
		
	@SuppressLint("SdCardPath")
	@Override
	public void onCreate(Bundle savedInstanceState){
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		final Button button1 = (Button) findViewById(R.id.web_button);
		final Button button2 = (Button) findViewById(R.id.stream_button);
		final Button button3 = (Button) findViewById(R.id.convert_button);
		final Button button4 = (Button) findViewById(R.id.play_button);
		final Button button5 = (Button) findViewById(R.id.http_button);

		button1.setOnClickListener(new View.OnClickListener() {
			
			//@Override
			public void onClick(View v) {
				
				String url = "http://people.sc.fsu.edu/~jburkardt/data/mp4/cavity_flow_movie.mp4";
				DownloadManager.Request request = new DownloadManager.Request(Uri.parse(url));
				request.setDescription("videomp4");
				request.setTitle("video");
				if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB) {
					
					//when downloading music and videos the will be listed in the player
					request.allowScanningByMediaScanner();

					//Notify user when download is complete
					request.setNotificationVisibility(
						DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED);
				}

				//Puts the download in the same Download directory the browser uses
				request.setDestinationInExternalPublicDir(
					Environment.DIRECTORY_DOWNLOADS, "cavity_flow_movie.mp4");

				manager = (DownloadManager) getSystemService(
					Context.DOWNLOAD_SERVICE);
				downloadReference = manager.enqueue(request);
				BroadcastReceiver receiver = new BroadcastReceiver() {
					@Override
					public void onReceive(Context context, Intent intent) {
						String action = intent.getAction();
						if (DownloadManager.ACTION_DOWNLOAD_COMPLETE.equals(action)) {
							//intent.getLongExtra(DownloadManager.EXTRA_DOWNLOAD_ID,0);
							Query myDownloadQuery = new Query();
							myDownloadQuery.setFilterById(downloadReference);
							Cursor myCursor = manager.query(myDownloadQuery);
							if (myCursor.moveToFirst()){
								int columnIndex = myCursor.getColumnIndex(DownloadManager.COLUMN_STATUS);
								if (DownloadManager.STATUS_SUCCESSFUL == myCursor.getInt(columnIndex)) {
									PlayVideo();
								}
							}
						}
					}
				};
				registerReceiver(receiver, new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE));
			}		
		});
		
		button2.setOnClickListener(new View.OnClickListener() {
			
			//@Override
			
			public void onClick(View v) {
				mPreview = (SurfaceView) findViewById(R.id.surface);
				holder = mPreview.getHolder();
				mediaPlayer = new MediaPlayer();
				try {
					mediaPlayer.setDataSource(getApplicationContext(), Uri.parse(
								Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
								+ "timati.mp4"));
				} catch (IllegalArgumentException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (SecurityException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (IllegalStateException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				
				mediaPlayer.setDisplay(holder);
				mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
				try {
					mediaPlayer.prepare();
				} catch (IllegalStateException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				mediaPlayer.start();
			}
		});
		
		button3.setOnClickListener(new View.OnClickListener() {
			
			//@Override
			
			public void onClick(View v) {
				
				thr2.start();
				thr1.start();
			}
		});
		
		button4.setOnClickListener(new View.OnClickListener() {
			
			//@Override
			
			public void onClick(View v) {
				
				thr1.start();
						
			}
		});
		
		button5.setOnClickListener(new View.OnClickListener() {
			
			//@Override
				
			public void onClick(View v) {
				
				thr3.start();
				thr4.start();
			}
		});
	}
	
	static {
		System.loadLibrary("ffmpeg");
		System.loadLibrary("mylib");
	}
	
	public static boolean isDownloadManagerAvailable(Context context){
		try {
			if (Build.VERSION.SDK_INT < Build.VERSION_CODES.GINGERBREAD){
				return false;
			}
			Intent intent = new Intent(Intent.ACTION_MAIN);
			intent.addCategory(Intent.CATEGORY_LAUNCHER);
			intent.setClassName("com.android.providers.downloads.ui", 
					"com.android.providers.downloads.ui.DownloadList");
			List<ResolveInfo> list = context.getPackageManager().queryIntentActivities(intent, PackageManager.MATCH_DEFAULT_ONLY);
			return list.size() > 0;
		} catch (Exception e) {
			return false;
		}
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}
	
	Runnable r1 = new Runnable() {
		public void run(){
			String[] port = {"6668"};
			String[] socket = {"http://127.0.0.1:6668/sdcard/Download/modified_video0.mp4"};
			
			httpserver.HttpStaticFileServer.main(port);
			try {
				httpserver.HttpSnoopClient.main(socket);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	};
	
	Thread thr1 = new Thread(r1);
	
	Runnable r2 = new Runnable() {
		public void run(){
			NumofLastDecFile = VideoDecoding(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS) + "/lexus" +
					".mp4",
					Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS) + "/modified_video.mp4");
			DecodingFlag = 1;
		}
	};
	
	Thread thr2 = new Thread(r2);
	
	Runnable r3 = new Runnable() {
		public void run(){
			Intent intent = new Intent(MainActivity.this,SDLActivity.class);
			startActivity(intent);
		}
	};
	
	Thread thr3 = new Thread(r3);
	
	Runnable r4 = new Runnable() {
		
		public void run(){
			while (MainActivity.ReceivedFlag != 1);
			EndCheck();
		}	
	};
	
	Thread thr4 = new Thread(r4);
	
	protected void PlayVideo() {
		
		mPreview = (SurfaceView) findViewById(R.id.surface);
		holder = mPreview.getHolder();
		mediaPlayer = new MediaPlayer();
		try {
			mediaPlayer.setDataSource(getApplicationContext(), Uri.parse(
					Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
					+ "/Frankenvini.2012.mp4"));
		} catch (IllegalArgumentException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (SecurityException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IllegalStateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		mediaPlayer.setDisplay(holder);
		mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
		try {
			mediaPlayer.prepare();
		} catch (IllegalStateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		mediaPlayer.start();
	}
	
	public class HttpClientServer extends AsyncTask<Object, Object, Object> {
		
		@Override
		protected Object doInBackground(Object... params) {
			
			String[] port = {"6668"};
			String[] socket = {"http://127.0.0.1:6668/sdcard/Download/modified_video0.mp4"};
			
			httpserver.HttpStaticFileServer.main(port);
			System.out.println("yoyoyoy");
			try {
				httpserver.HttpSnoopClient.main(socket);
			} catch (Exception e) {
				e.printStackTrace();
			}
			
			return null;
		}
		
	}
	
	//I start server at PC side
	
	public void Startserver(){
		
		ServerBootstrap bootstrap = new ServerBootstrap(
				new NioServerSocketChannelFactory(
						Executors.newCachedThreadPool(),
						Executors.newCachedThreadPool()));
		
		bootstrap.setPipelineFactory(new ChannelPipelineFactory() {
			 public ChannelPipeline getPipeline() throws Exception {
				 return Channels.pipeline(
						 new ObjectDecoder(ClassResolvers.cacheDisabled(getClass().getClassLoader())),
						 new ObjectEncoder(),
						 new ServerDateHandler());
			 };
		});
		
		bootstrap.bind(new InetSocketAddress("127.0.0.1", 6666));	
		System.out.println("Listening on 8080");
	}
	
	//Handler for received data
	
	static class ServerDateHandler extends SimpleChannelHandler {
		Random random = new Random(System.nanoTime());
		public void messageReceived(ChannelHandlerContext ctx,MessageEvent e) throws Exception {
			Date date = (Date)e.getMessage();
			// Here's the REALLY important business service at the end of the pipeline
			long newTime = (date.getTime() + random.nextInt());
			Date newDate = new Date(newTime);
			System.out.println("Hey Guys !  I got a date ! [" + date + "] and I modofoed it to [" + newDate + "]");
			Channel channel = e.getChannel();
			ChannelFuture channelFuture = Channels.future(e.getChannel());
			ChannelEvent responseEvent = new DownstreamMessageEvent(channel, channelFuture, newDate, channel.getRemoteAddress());
			ctx.sendDownstream(responseEvent);
			// But still send it upstream because there might be another handler
			super.messageReceived(ctx, e);
		}  
	}
	
	public void StartClient(){
		
		Executor bossPool = Executors.newCachedThreadPool();
		Executor workerPool = Executors.newCachedThreadPool();
		ChannelFactory channelFactory = new NioClientSocketChannelFactory(bossPool,workerPool);
		ChannelPipelineFactory pipelineFactory = new ChannelPipelineFactory() {
			public ChannelPipeline getPipeline() throws Exception {
				return Channels.pipeline(new ObjectEncoder(),
										 new ObjectDecoder(ClassResolvers.cacheDisabled(getClass().getClassLoader())),
										 new ClientDateHandler());
			}
		};
		
		Bootstrap bootstrap = new ClientBootstrap(channelFactory);
		bootstrap.setPipelineFactory(pipelineFactory);
		InetSocketAddress addressToConnectTo = new InetSocketAddress("127.0.0.1",6666);	
		
//		Map<String, Object> options = new HashMap<String, Object>();
//		options.put("connectTimeoutMillis", 2000);
//		SimpleNIOClient client = new SimpleNIOClient(options);
		ChannelFuture cf = ((ClientBootstrap) bootstrap).connect(addressToConnectTo);
		cf.awaitUninterruptibly();
		cf.addListener(new ChannelFutureListener(){
			public void operationComplete(ChannelFuture future) throws Exception{
				if(future.isSuccess()){
					Channel channel = future.getChannel();
					channel.write(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS) + "/modified_video.mp4");
				}
			}
		});
		
		Channel channel = cf.getChannel();
		channel.write(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS) + "/modified_video.mp4");
	}
	
	static class ClientDateHandler extends SimpleChannelHandler {
		public void messageReceived(ChannelHandlerContext ctx, MessageEvent e) throws Exception {
			Date date = (Date)e.getMessage();
			System.out.println("I got a modified date! [" + date + "]");
		}
	}

}