package com.example.player

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity

class ScreenLiveActivity : AppCompatActivity() {
	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		setContentView(R.layout.activity_screen_live)

	}

	override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
		super.onActivityResult(requestCode, resultCode, data)
	}


}