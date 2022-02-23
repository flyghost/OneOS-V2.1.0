
stat = process.memstat();
console.log("memstat:");
console.log("  total :",stat.total);
console.log("  allocated :",stat.allocated);
console.log("  peak :",stat.peak);
