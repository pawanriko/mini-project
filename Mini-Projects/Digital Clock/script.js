function updateClock() {
    const now = new Date();

    let hours = String(now.getHours()).padStart(2, "0");
    let minutes = String(now.getMinutes()).padStart(2, "0");
    let seconds = String(now.getSeconds()).padStart(2, "0");

    document.getElementById("time").textContent =
        `${hours}:${minutes}:${seconds}`;
}

setInterval(updateClock, 1000);

updateClock();