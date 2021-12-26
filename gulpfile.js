const fs = require('fs');
const gulp = require('gulp');
const htmlmin = require('gulp-htmlmin');
const fontmin = require('gulp-fontmin');
const concat = require('gulp-concat');
const cleancss = require('gulp-clean-css');
const purgecss = require('gulp-css-purge');
const uglify = require('gulp-uglify');
const gzip = require('gulp-gzip');
const del = require('del');
const inline = require('gulp-inline');
const inlineImages = require('gulp-css-base64');
const favicon = require('gulp-base64-favicon');
const inlineFonts = require('gulp-inline-fonts');
const { doesNotMatch, doesNotReject } = require('assert');

const dataFolder = 'src/web/';
 
gulp.task('clean', () => {
    return del([ dataFolder + '*']);
});

gulp.task('icons', gulp.series("clean", () => {
    return gulp.src(['html/font/webfonts/fa-solid-*.ttf'])
        .pipe(fontmin({
            text: ``
        }))
        .pipe(inlineFonts({
            name: 'fa5',
            style: "normal",
            weight: 900,
            formats: ['ttf']
        }))
        .pipe(gulp.dest('html/font/'));
}));

gulp.task('buildfs_inline', gulp.series('icons', () => {
    return gulp.src('html/*.html')
        .pipe(favicon())
        .pipe(inline({
            base: 'html/',
            js: uglify,
            css: [
                cleancss,
                inlineImages
            ],
            disabledTypes: ['svg' ]
        }))
        .pipe(htmlmin({
            collapseInlineTagWhitespace: true,
            removeEmptyAttributes: true,
            collapseWhitespace: true,
            removeComments: true,
            minifyCSS: {
                level: 2
            },
            minifyJS: true
        }))
        .pipe(gzip({
            gzipOptions: {
                level: 9
            }
        }))
        .pipe(gulp.dest(dataFolder));
}));

gulp.task('buildfs', gulp.series("buildfs_inline", (done) => {
    const source = dataFolder + 'index.html.gz';
    const destination = dataFolder + 'index.html.gz.h';

    const wstream = fs.createWriteStream(destination);
    wstream.on('error', (err) => {
        console.log(err);
    });

    const data = fs.readFileSync(source);

    wstream.write('#define index_html_gz_len ' + data.length + '\n');
    wstream.write('const uint8_t index_html_gz[] PROGMEM = {');

    for (i=0; i<data.length; i++) {
        if (i % 1000 === 0) wstream.write("\n");
        wstream.write('0x' + ('00' + data[i].toString(16)).slice(-2));
        if (i<data.length-1) wstream.write(',');
    }

    wstream.write('\n};');
    wstream.end();

    done();
}));

gulp.task('default', gulp.series('buildfs'));
