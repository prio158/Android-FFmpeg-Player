package com.example.player.common

import android.annotation.SuppressLint
import android.util.Log
import android.view.Surface


typealias Callback = (code: Int) -> Unit
typealias EmptyCallback = () -> Unit

class Player {

	@SuppressLint("UnsafeDynamicallyLoadedCode")
	companion object {
		private const val TAG = "PlayerKT"

		init {
			System.loadLibrary("player")
		}
	}

	private var nativePlayer = nativePlayerInit()
	private var onErrorCallback: Callback = {}
	private var onProcessCallback: Callback = {}
	private var onPrepareCallback: EmptyCallback = {}

	fun setSurface(surface: Surface) {
		setSurface(nativePlayer, surface)
	}

	fun setDataSource(path: String) {
		setDataSource(nativePlayer, path)
	}

	fun start() {
		nativeStart(nativePlayer)
	}

	fun stop() {
		nativeStop(nativePlayer)
	}

	/**
	 * @brief 获取媒体文件信息，准备解码器
	 * */
	fun prepare() {
		nativePrepare(nativePlayer)
	}

	/**
	 * @brief 设置Native层的回调方法
	 * @param onErrorCallback_ Native发生错误时的回调方法
	 * @param onPrepareCallback_ Native Prepare阶段完成的回调
	 * @param onProgressCallback_ Native 播放进度发生变化时的回调
	 * */
	fun setOnNativeCallback(
		onErrorCallback_: Callback = {},
		onPrepareCallback_: EmptyCallback = {},
		onProgressCallback_: Callback = {}
	) {
		onErrorCallback = onErrorCallback_
		onPrepareCallback = onPrepareCallback_
		onProcessCallback = onProgressCallback_
	}

	/**
	 * @brief Native出错触发的回调方法
	 * */
	private fun onError(code: Int) {
		Log.e(TAG, "onError:Call")
		onErrorCallback.invoke(code)
	}

	/**
	 * @brief Native Prepare完成触发的回调方法
	 * */
	private fun onPrepare() {
		Log.i(TAG, "onPrepare:Call")
		onPrepareCallback.invoke()
	}

	/**
	 * @brief Native 播放进度的回调方法
	 * */
	private fun onProgress(progress: Int) {
		Log.i(TAG, "onProgress:Call")
		onProcessCallback.invoke(progress);
	}

	/**
	 * @brief 在Native创建Player
	 * @return 返回Native的Player指针到Java层
	 * */
	private external fun nativePlayerInit(): Long

	private external fun setDataSource(nativePtr: Long, path: String)

	private external fun nativePrepare(nativePtr: Long)

	private external fun nativeStart(nativePtr: Long)

	private external fun nativeStop(nativePtr: Long)

	private external fun nativeEnable(nativePtr: Long)

	private external fun setSurface(nativePtr: Long, surface: Surface)

}