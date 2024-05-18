package com.example.player.common

import java.util.concurrent.LinkedBlockingQueue
import java.util.concurrent.ThreadPoolExecutor
import java.util.concurrent.TimeUnit

object LiveThreadPool {

	private val CPU_COUNT = Runtime.getRuntime().availableProcessors()

	private val CORE_POOL_SIZE = 2.coerceAtLeast((CPU_COUNT - 1).coerceAtMost(4))

	private val MAXIMUM_POOL_SIZE = CPU_COUNT * 2 + 1

	private const val KEEP_ALIVE_SECONDS = 30L

	private val sPoolWorkQueue = LinkedBlockingQueue<Runnable>(5)

	private val executor = ThreadPoolExecutor(
		CORE_POOL_SIZE, MAXIMUM_POOL_SIZE, KEEP_ALIVE_SECONDS, TimeUnit.SECONDS,
		sPoolWorkQueue
	).apply {
		this.allowCoreThreadTimeOut(true)
	}

	fun execute(runnable: Runnable) = executor.execute(runnable)

}