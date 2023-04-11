<h1>ZeroThreadKernel</h1>
<p>ZeroThreadKernel allows you to execute arbitrary code recursively at kernel-level without a system thread.</p>
<h2>How It Works</h2>
<p>It works by hooking a non-PatchGuard protected function exported by the DirectX graphics kernel subsystem (dxgkrnl). The function is called in our user-mode program using the export from win32u.dll, which serves as a thread for recursive code execution.</p>
<h2>Detection</h2>
<p>One way to detect is by integrity checking the .text section of the specified module. Writing our shellcode in the function modifies the original hash of the module.</p>
<h2>Contributing</h2>
<p>Contributions are always welcome!</p>
<h2>Demo</h2>

![Demo](demo.gif)
