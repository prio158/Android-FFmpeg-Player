package coroutine

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.withContext
import java.util.concurrent.CancellationException
import kotlin.coroutines.CoroutineContext


/**
 * runCatching在 coroutine中使用
 * */
inline fun <T> runCancelableCatching(block: () -> T): Result<T> =
	try {
		val result = block()
		Result.success(result)
	} catch (e: CancellationException) {
		throw e
	} catch (e: Exception) {
		Result.failure(e)
	}

inline fun <T, R> T.runCancelableCatching(block: T.() -> R): Result<R> =
	try {
		val result = block()
		Result.success(result)
	} catch (e: CancellationException) {
		throw e
	} catch (e: Exception) {
		Result.failure(e)
	}

suspend inline operator fun <T> CoroutineContext.invoke(
	noinline block: suspend CoroutineScope.() -> T
): T = withContext(this, block)