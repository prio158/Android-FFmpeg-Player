package com.example.player.common

import android.content.Context
import android.media.projection.MediaProjectionManager
import androidx.appcompat.app.AppCompatActivity

class ScreenLive {

	private var url: String? = null

	/**
	 * 开始推流
	 * */
	fun startLive(url: String, activity: AppCompatActivity) {

		this.url = url

		val manager =
			activity.getSystemService(Context.MEDIA_PROJECTION_SERVICE) as? MediaProjectionManager

		manager?.apply {
			val intent = this.createScreenCaptureIntent()
			activity.startActivityForResult(intent, 100)

		}

	}


	fun stopLive() {

	}


}