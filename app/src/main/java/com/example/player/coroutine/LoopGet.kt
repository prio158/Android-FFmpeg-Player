package coroutine

import kotlinx.coroutines.*
import kotlin.properties.Delegates


suspend fun loopCheck(
	timeOutMillis: Long = 5_000,
	periodMillis: Long = 200,
	onStart: () -> Unit = onStartStub,
	onSuccess: (result: Unit, executeDuration: Long, executeCount: Int) -> Unit = onSuccessStub,
	onError: (error: Throwable, executeDuration: Long, executeCount: Int) -> Unit = onErrorStub,
	onEach: (newCount: Int) -> Unit = onEachStub,
	predicate: suspend CoroutineScope.() -> Boolean
): Boolean = try {
	val block: suspend CoroutineScope.() -> Unit? = {
		if (predicate(this)) Unit else null
	}
	loopGet(timeOutMillis, periodMillis, block, onStart, onSuccess, onError, onEach)
	true
} catch (e: TimeoutCancellationException) {
	false
}


suspend fun loopCheck(
	timeOutMillis: Long = 5_000,
	periodMillis: Long = 200,
	tracker: LoopGetTracker<Unit> = LoopGetTracker.empty(),
	predicate: suspend CoroutineScope.() -> Boolean
): Boolean = try {
	val block: suspend CoroutineScope.() -> Unit? = {
		if (predicate(this)) Unit else null
	}
	loopGet(timeOutMillis, periodMillis, tracker, block)
	true
} catch (e: TimeoutCancellationException) {
	false
}


suspend fun <T> loopGet(
	timeOutMillis: Long = 5_000,
	periodMillis: Long = 200,
	block: suspend CoroutineScope.() -> T?,
	onStart: () -> Unit = onStartStub,
	onSuccess: (result: T, executeDuration: Long, executeCount: Int) -> Unit = onSuccessStub,
	onError: (error: Throwable, executeDuration: Long, executeCount: Int) -> Unit = onErrorStub,
	onEach: (newCount: Int) -> Unit = onEachStub
): T = loopGet(
	timeOutMillis = timeOutMillis,
	periodMillis = periodMillis,
	tracker = object : LoopGetTracker<T> {
		override fun onStart() = onStart()
		override fun onSuccess(result: T, executeDuration: Long, executeCount: Int) {
			onSuccess(result, executeDuration, executeCount)
		}

		override fun onError(error: Throwable, executeDuration: Long, executeCount: Int) {
			onError(error, executeDuration, executeCount)
		}

		override fun onEach(newCount: Int) = onEach(newCount)
	},
	block = block
)

suspend fun <T> loopGet(
	timeOutMillis: Long = 5_000,
	periodMillis: Long = 200,
	tracker: LoopGetTracker<T> = LoopGetTracker.empty(),
	block: suspend CoroutineScope.() -> T?
): T = trackExecute(tracker) {
	withTimeout(timeOutMillis) {
		var result: T? = null
		while (isActive) {
			it.count++
			result = block(this)
			if (null != result)
				break
			else
				delay(periodMillis)
		}
		ensureActive()
		result!!
	}
}

private inline fun <T> trackExecute(
	tracker: LoopGetTracker<T>,
	block: (WhileCounter) -> T
): T {
	val whileCounter = WhileCounter(tracker::onEach)
	val start = System.currentTimeMillis()
	return try {
		tracker.onStart()
		val ret = block(whileCounter)
		val executeDuration = System.currentTimeMillis() - start
		tracker.onSuccess(ret, executeDuration, whileCounter.count)
		ret
	} catch (e: Exception) {
		val executeDuration = System.currentTimeMillis() - start
		tracker.onError(e, executeDuration, whileCounter.count)
		throw e
	}
}

private val onStartStub = {}
private val onSuccessStub = { _: Any?, _: Long, _: Int -> }
private val onErrorStub = { _: Throwable, _: Long, _: Int -> }
private val onEachStub = { _: Int -> }

interface LoopGetTracker<T> {

	fun onStart() {}

	fun onSuccess(result: T, executeDuration: Long, executeCount: Int) {}

	fun onError(error: Throwable, executeDuration: Long, executeCount: Int) {}

	fun onEach(newCount: Int) {}

	private object EMPTY : LoopGetTracker<Any?>

	companion object {
		@Suppress("UNCHECKED_CAST")
		fun <T> empty(): LoopGetTracker<T> = EMPTY as LoopGetTracker<T>
	}
}

private class WhileCounter(countChangeListener: (newCount: Int) -> Unit) {
	var count: Int by Delegates.observable(0) { _, _, newValue -> countChangeListener(newValue) }
}