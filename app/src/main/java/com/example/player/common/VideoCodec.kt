package com.example.player.common

import android.content.Context
import android.hardware.display.DisplayManager
import android.hardware.display.VirtualDisplay
import android.media.MediaCodec
import android.media.MediaCodecInfo
import android.media.MediaFormat
import android.media.projection.MediaProjection
import android.os.Bundle
import android.util.Log
import kotlin.math.abs

class VideoCodec(val context: Context) : Thread() {

	private val screenWidth = context.resources.displayMetrics.widthPixels
	private val height = 720
	private lateinit var mediaCodec: MediaCodec
	private var isLiving = false
	private var refreshTimStamp = 0L
	private var mediaProjection: MediaProjection? = null
	private var virtualDisplay: VirtualDisplay? = null

	companion object {
		private const val TAG = "VideoCodecTAG"
	}

	fun startLive(mediaProjection: MediaProjection) {
		isLiving = true
		this.mediaProjection = mediaProjection
		initCodec()
		start()
	}

	private fun initCodec() {
		val mediaFormat =
			MediaFormat.createVideoFormat(MediaFormat.MIMETYPE_VIDEO_AVC, screenWidth, height)

		mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, 400_000)
		//下面就相当于，每编码 50帧，来一个 I 帧数据
		mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, 25)
		mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 2) //2s
		mediaFormat.setInteger(
			MediaFormat.KEY_COLOR_FORMAT,
			MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface
		)
		mediaCodec = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_VIDEO_AVC)
		mediaCodec.configure(
			mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE
		)
		val inputSurface = mediaCodec.createInputSurface()
		virtualDisplay = mediaProjection?.createVirtualDisplay(
			"screen-codec",
			screenWidth, height, 1,
			DisplayManager.VIRTUAL_DISPLAY_FLAG_PUBLIC,
			inputSurface, null, null
		)
	}


	override fun run() {
		super.run()
		mediaCodec.start()
		val bufferInfo = MediaCodec.BufferInfo()
		while (isLiving) {
			if (refreshTimStamp != 0L) {
				val currentTime = System.currentTimeMillis()
				if (abs(refreshTimStamp - currentTime) >= 2_000) {
					refreshTimStamp = currentTime
					Bundle().apply {
						//手动设置MediaCodec，让其他每两秒编码一个关键帧
						putInt(MediaCodec.PARAMETER_KEY_REQUEST_SYNC_FRAME, 0)
						mediaCodec.setParameters(this)
					}
				}
			} else {
				Log.i(TAG, "第一帧")
				refreshTimStamp = System.currentTimeMillis()
			}

			//获取编码后的数据
			val index = mediaCodec.dequeueOutputBuffer(bufferInfo, 200)
			if (index >= 0) {
				//成功取出数据
				val byteBuffer = mediaCodec.getOutputBuffer(index)
				val data = ByteArray(bufferInfo.size)
				byteBuffer?.get(data) //取出编码好的数据
				//将编码后的数据 data 按照 rtmp 封包的格式发送出去

				//释放队列index的位置
				mediaCodec.releaseOutputBuffer(index, false)
			} else {
				Log.e(TAG, "dequeueOutputBuffer:${index}")
			}

		}
		isLiving = false
		mediaCodec.stop()
		mediaCodec.release()
		virtualDisplay?.release()
		mediaProjection?.stop()
	}

}