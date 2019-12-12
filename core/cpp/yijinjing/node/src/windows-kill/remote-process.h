#pragma once
#include "signal.h"
#include "ctrl-routine.h"

namespace WindowsKillLibrary {
	class RemoteProcess
	{
	private:
		/// <summary>
		/// The needed access for open remote process.
		/// </summary>
		static const DWORD NEEDEDACCESS = PROCESS_QUERY_INFORMATION | PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD;
		
		/// <summary>
		/// The that we want to send.
		/// </summary>
		Signal* the_signal;

		/// <summary>
		/// The control routine corresponding to the signal.
		/// </summary>
		CtrlRoutine* the_ctrl_routine;

		/// <summary>
		/// The process token handle.
		/// </summary>
		HANDLE process_token;

		/// <summary>
		/// The remote process handle.
		/// </summary>
		HANDLE handle;

		/// <summary>
		/// The remote thread handle.
		/// </summary>
		HANDLE remote_thread;

		/// <summary>
		/// Closes the opend process token by handle.
		/// </summary>
		void closeProcessToken(void);

		/// <summary>
		/// Sets the needed privilege for opening the remote process.
		/// </summary>
		/// <param name="enable_privilege">if set to <c>true</c> [enable privilege].</param>
		/// <returns></returns>
		bool setPrivilege(bool enable_privilege);

		/// <summary>
		/// Closes the opend remote thread by handle.
		/// </summary>
		void closeRemoteThread(void);

		/// <summary>
		/// Closes the opend handle.
		/// </summary>
		void closeHandle(void);
	public:
		/// <summary>
		/// Initializes a new instance of the <see cref="RemoteProcess"/> class.
		/// </summary>
		RemoteProcess();

		/// <summary>
		/// Sets the Signal that we want to send.
		/// </summary>
		/// <param name="the_signal">The signal.</param>
		void setSignal(Signal* the_signal);

		/// <summary>
		/// Sets the ctrl routine corresponding to the signal.
		/// </summary>
		/// <param name="the_ctrl_routine">The control routine.</param>
		void setCtrlRoutine(CtrlRoutine* the_ctrl_routine);

		/// <summary>
		/// Opens the remote process.
		/// </summary>
		void open(void);

		/// <summary>
		/// Starts the remote thread and trigger ctrl routine by address.
		/// </summary>
		void startRemoteThread(void);
	};
}
