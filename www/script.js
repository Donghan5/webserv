// function setCookie() {
//     fetch("/set-cookie", { method: "GET", credentials: "same-origin" })
//     .then(response => response.text())
//     .then(data => {
//         document.getElementById("cookie-status").innerText = "✅ Cookie Set!";
//     })
//     .catch(error => {
//         document.getElementById("cookie-status").innerText = "❌ Failed to set cookie!";
//     });
// }



// // Function to get a cookie
// function getCookie() {
//     fetch("/get-cookie", {
//         method: "GET",
//         credentials: "include"
//     })
//     .then(response => response.text())
//     .then(data => {
//         document.getElementById("cookie-status").innerText = "🍪 Cookie: " + data;
//     })
//     .catch(error => {
//         document.getElementById("cookie-status").innerText = "❌ No cookie found!";
//     });
// }


// Track visit count
window.onload = function() {
    let visits = getCookie("visits") || 0;
    visits++;
    setCookie("visits", visits, 7);
    document.getElementById("visit-count").innerText = `Visit count: ${visits}`;
};

function sayHello() {
    const button = document.querySelector(".btn-primary");
    button.innerText = "✅ 클릭 완료!";
    button.style.background = "#4CAF50";
    button.style.color = "white";

    setTimeout(() => {
        button.innerText = "클릭해보세요";
        button.style.background = "#ffcc00";
        button.style.color = "black";
    }, 2000);

    alert("Hello! Welcome to webserv!!");
}
