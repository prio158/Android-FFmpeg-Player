package com.example.player.common

import android.app.Activity
import android.content.Context
import android.content.Intent
import android.hardware.display.DisplayManager
import android.media.projection.MediaProjection
import android.media.projection.MediaProjectionManager
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity

class ScreenLive(private val context: Context) : Runnable {

	private lateinit var url: String
	private lateinit var manager: MediaProjectionManager
	private lateinit var mediaProjection: MediaProjection

	/**
	 * 开始推流
	 * */
	fun startLive(url: String, activity: AppCompatActivity) {
		this.url = url
		manager =
			activity.getSystemService(Context.MEDIA_PROJECTION_SERVICE) as MediaProjectionManager
		manager.apply {
			val intent = this.createScreenCaptureIntent()
			activity.startActivityForResult(intent, 100)
		}
	}

	fun stopLive() {

	}

	fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent) {
		if (requestCode == 100 && resultCode == Activity.RESULT_OK) {
			mediaProjection = manager.getMediaProjection(resultCode, data)
			LiveThreadPool.execute(this)
		}
	}

	override fun run() {
		if (!connect(url)) {
			//连接失败
			return
		}
		val videoCodec = VideoCodec(context)
		videoCodec.startLive(mediaProjection)
	}

	private external fun connect(url: String?): Boolean


}