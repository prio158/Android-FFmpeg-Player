package com.example.player

import android.content.Intent
import android.net.Uri
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.provider.Settings
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import androidx.core.app.ActivityCompat
import com.example.player.common.Player
import com.example.player.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

	private lateinit var binding: ActivityMainBinding
	private lateinit var player: Player
	private var isRefuse: Boolean = false
	private var surface: Surface? = null
	private var isPause = false

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)

		binding = ActivityMainBinding.inflate(layoutInflater)

		setContentView(binding.root)

		verifyStoragePermissions()

		player = Player()

		player.setDataSource("/storage/emulated/0/guabi.mp4")

		/***
		 *  Surface通常由图像缓冲区的消费者来创建,然后被移交给生产者（eg:MediaPlayer）,
		 *  然后生产者将数据填充到Surface上
		 *
		 *  SurfaceView 提供了嵌入视图层级中的专用surface。你可以控制
		 *  surface 的格式或大小。SurfaceView负责把surface显示在屏幕
		 *  的正确位置。
		 *
		 *  SurfaceView 继承自View，其中有两个成员变量，一个是Surface对象，一个
		 *  是SurfaceHolder 对象
		 *
		 *  SurfaceView 通过SurfaceHolder 告诉我们Surface的状态（创建、变化、销毁）
		 *
		 * */
		val surfaceView = binding.surfaceView
		/**
		 * 一个抽象接口，给持有surface的对象使用，它可以控制surface
		 * 的大小和格式，编辑surface中的像素，以及监听surface的变化
		 * */
		surfaceView.holder.addCallback(object : SurfaceHolder.Callback {
			override fun surfaceCreated(holder: SurfaceHolder) {
				Log.i(TAG, "surfaceCreated:第一次创建时回调")
			}

			override fun surfaceChanged(
				holder: SurfaceHolder,
				format: Int,
				width: Int,
				height: Int
			) {
				Log.i(TAG, "surfaceChanged: surface 变化的时候回调")
				surface = holder.surface
			}

			override fun surfaceDestroyed(holder: SurfaceHolder) {
				Log.i(TAG, "surfaceDestroyed: surface 销毁的时候回调")
			}

		})

		player.setOnNativeCallback(
			onPrepareCallback_ = {
				Log.d(TAG_PLAY, "player.start()")
				//此处能保证VideoChannel初始化完毕
				surface?.let { player.setSurface(it) }
				player.start()
			},
			onProgressCallback_ = {

			},
			onErrorCallback_ = {

			}
		)

		binding.button.setOnClickListener {
			player.prepare()
		}

		binding.buttonPause.setOnClickListener {
			isPause = !isPause
			if (isPause)
				player.pause()
			else
				player.continuePlay()
			binding.buttonPause.text = if (!isPause) "Pasue" else "Continue"
		}
	}

	private fun verifyStoragePermissions() {
		if (!Environment.isExternalStorageManager() && !isRefuse) {
			val intent = Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
			intent.data = Uri.parse("package:$packageName");
			ActivityCompat.startActivityForResult(this, intent, 1024, Bundle())
		}
	}

	@Deprecated("Deprecated in Java")
	override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
		super.onActivityResult(requestCode, resultCode, data)
		if (requestCode == 1024) {
			// 检查是否有权限
			isRefuse = !Environment.isExternalStorageManager()
			Log.i(TAG_FILE, "onActivityResult:拒绝权限")
		}
	}

	companion object {
		const val TAG = "SurfaceView"
		const val TAG_FILE = "FILE"
		const val TAG_PLAY = "TAG_PLAY"

	}
}