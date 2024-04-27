package coroutine

import androidx.fragment.app.Fragment
import androidx.lifecycle.*
import kotlinx.coroutines.*


val LifecycleOwner.lifecycleSafeScope: LifecycleCoroutineSafeScope
	get() = when (this) {
		is Fragment -> viewLifecycleOwner
		else -> this
	}.safeScope()

val ViewModel.viewModelSafeScope: CoroutineScope
	get() = viewModelScope + DefaultCoroutineExceptionHandler

val applicationSafeScope: LifecycleCoroutineSafeScope
	get() = ProcessLifecycleOwner.get().lifecycleSafeScope

interface LifecycleCoroutineSafeScope : CoroutineScope {
	fun launchWhenCreated(block: suspend CoroutineScope.() -> Unit): Job
	fun launchWhenStarted(block: suspend CoroutineScope.() -> Unit): Job
	fun launchWhenResumed(block: suspend CoroutineScope.() -> Unit): Job
}

/** coroutine异常处理器*/
private object DefaultCoroutineExceptionHandler : CoroutineExceptionHandler
by CoroutineExceptionHandler(
	handler = { coroutineContext, error ->
		val errorMsg =
			"[coroutine-error:${Thread.currentThread().name} ${coroutineContext[Job]}] ${error.message}"
		val exceptionData = Exception(errorMsg)
		throw exceptionData
	}
)

private fun LifecycleOwner.safeScope(): LifecycleCoroutineSafeScope =
	LifecycleCoroutineSafeScopeImpl(lifecycle)

private class LifecycleCoroutineSafeScopeImpl(
	private val lifecycle: Lifecycle
) : LifecycleCoroutineSafeScope,
	CoroutineScope by lifecycle.coroutineScope + DefaultCoroutineExceptionHandler {
	override fun launchWhenCreated(block: suspend CoroutineScope.() -> Unit): Job = launch {
		lifecycle.whenCreated(block)
	}

	override fun launchWhenStarted(block: suspend CoroutineScope.() -> Unit) = launch {
		lifecycle.whenStarted(block)
	}

	override fun launchWhenResumed(block: suspend CoroutineScope.() -> Unit) = launch {
		lifecycle.whenResumed(block)
	}
}