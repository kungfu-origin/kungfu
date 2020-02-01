#pragma once

namespace WindowsKillLibrary {
	/// <summary>
	/// Signal class to store and validate the signal we want to send.
	/// </summary>
	class Signal
	{
	private:
		/// <summary>
		/// The process id (pid) that this signal will be sent to.
		/// </summary>
		DWORD pid;

		/// <summary>
		/// Signal type.
		/// Types:
		///		1) 0 CTRL_C		SIGINT
		///		2) 1 CTRL_BREAK	SIGBREAK
		/// </summary>
		DWORD_PTR type;

		/// <summary>
		/// Validates the type of signal to see if supported.
		/// </summary>
		/// <param name="type">type</param>
		static void validateType(DWORD_PTR type);

		/// <summary>
		/// Checks that process with given pid exist.
		/// </summary>
		/// <param name="pid">The pid.</param>
		static void checkPidExist(DWORD pid);
	public:
		/// <summary>
		/// Initializes a new instance of the <see cref="Signal"/> class.
		/// </summary>
		/// <param name="pid">Target process id</param>
		/// <param name="type">type</param>
		Signal(DWORD pid, DWORD_PTR type);

		/// <summary>
		/// Gets the type of signal.
		/// </summary>
		/// <returns>Signal's type</returns>
		DWORD_PTR getType(void);

		/// <summary>
		/// Gets the pid of target process.
		/// </summary>
		/// <returns>Target process id</returns>
		DWORD getPid(void);
	};
}

