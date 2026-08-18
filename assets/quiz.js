function checkQuiz(id, answers) {
  var input = document.getElementById(id + "-input");
  var fb = document.getElementById(id + "-feedback");
  var given = input.value.trim().toLowerCase();
  var norm = answers.map(function (a) { return a.trim().toLowerCase(); });
  var ok = norm.indexOf(given) !== -1;
  fb.style.display = "block";
  fb.className = "feedback " + (ok ? "correct" : "incorrect");
  fb.textContent = ok ? "Correct." : "Not quite — re-check the slot above, then try again.";
}
